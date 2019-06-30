#include "nwol_array.h"

#include "ftplib.h"

#include <stdio.h>
#include <string>
#include <Windows.h>

static ::nwol::array_obj<std::string>	logLines;

#define log_printf(...)																	\
{																						\
	info_printf(__VA_ARGS__);															\
	char_t										tmpStr[512]				= {};			\
	sprintf_s(tmpStr, __VA_ARGS__);														\
	printf("%s", tmpStr);																\
	logLines.push_back(tmpStr);															\
}

int32_t									getLines				(const char* source, int32_t maxLen, ::nwol::array_obj<std::string>& lines_)	{
	::nwol::array_obj<std::string>				lines;
	int32_t										bufferIndex				= 0;
	while(bufferIndex < maxLen) {
		char										nameBuffer[256]			= {};
		int32_t										nameLen					= 0;
		while(nameLen < 256 && bufferIndex < maxLen) {
			nameBuffer[nameLen++]					= source[bufferIndex++];
			if(nameBuffer[nameLen - 1] == '\n') {
				if(nameLen > 1 && nameBuffer[nameLen - 2] == '\r')
					nameBuffer[nameLen - 2]					= 0;
				else
					nameBuffer[nameLen - 1]					= 0;
				break;
			}
		}

		lines.push_back(nameBuffer);
	}

	lines_									= lines;

	return 0;
}

static const char						tempFileName[]			= "launcher.tmp";

int32_t									listRemote				(netbuf* netConnection, ::nwol::array_obj<std::string>& fileNames_)		{
	int											success					= 0;

	const char									remotePath[]			= "/";// ".gitignore";
	success									= ::FtpNlst(tempFileName, remotePath, netConnection);

	FILE										* fp					= 0;
	int32_t										error					= fopen_s(&fp, tempFileName, "rb");
	if(0 == fp || 0 != error)
		return -1;
	fseek(fp, 0, SEEK_END);
	int32_t										fileSize				= ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char										* buffer				= (char*)malloc(fileSize+1);
	fread(buffer, 1, fileSize, fp);
	fclose(fp);
	buffer[fileSize]						= 0;

	::nwol::array_obj<::std::string>			fileNames;
	::getLines(buffer, fileSize, fileNames_);

	free(buffer);
	return 0;
}

#pragma pack(push, 1)
struct SPatchDate {
	int32_t								PackedDate;
	int16_t								Year;
	char								Month;
	char								Day;
};

struct SPatchFile : public SPatchDate {
	::std::string						Path;
};

struct SLauncherSettings {
	int32_t								PrevDate				= 0;
	int32_t								LastDate				= 0;
	::nwol::array_obj<SPatchFile>		Files					= {};
};
#pragma pack(pop)


int32_t									loadSettings			(SLauncherSettings& launcherSettings, const char* settingsFileName)			{
	FILE										* fp					= 0;
	fopen_s(&fp, settingsFileName, "rb");
	if(fp) {
		fseek(fp, 0, SEEK_END);
		int32_t										fileSize				= ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fread(&launcherSettings, 1, sizeof(int64_t) > fileSize ? fileSize : sizeof(int64_t), fp);
		
		uint32_t									fileCount				= 0;
		fread(&fileCount, 1, sizeof(uint32_t), fp);
		launcherSettings.Files.resize(fileCount);
		for(uint32_t i = 0; i < fileCount; ++i) {
			SPatchDate									* fileDate				= &launcherSettings.Files[i];	fread(fileDate, 1, sizeof(SPatchDate), fp);
			int32_t										nameLen					= 0;							fread(&nameLen, 1, sizeof(int32_t), fp);
			char										path[256]				= {};							fread(&path[0], 1, nameLen > 256 ? 256 : nameLen, fp);
			launcherSettings.Files[i].Path			= path;
		}
		fclose(fp);
	}
	else
		return -1;

	return 0;
}

int32_t									saveSettings			(const SLauncherSettings& launcherSettings, const char* settingsFileName)	{
	FILE										* fp					= 0;
	fopen_s(&fp, settingsFileName, "wb");
	if(fp) {
		fwrite(&launcherSettings, 1, sizeof(int64_t), fp);
		uint32_t									fileCount				= (uint32_t)launcherSettings.Files.size();
		fwrite(&fileCount, 1, sizeof(uint32_t), fp);
		for(uint32_t i=0; i<fileCount; ++i) {
			const SPatchDate							* fileDate				= &launcherSettings.Files[i];
			fwrite(fileDate, 1, sizeof(SPatchDate), fp);
			int32_t										nameLen					= (int32_t)launcherSettings.Files[i].Path.size();
			fwrite(&nameLen, 1, sizeof(int32_t), fp);
			fwrite(launcherSettings.Files[i].Path.c_str(), 1, nameLen, fp);
		}
		fclose(fp);
	}
	else
		return -1;

	return 0;
}

template <size_t _Size>
int32_t									loadHost				(char (&patchhost)[_Size])	{
	const char									hostFileName[]			= "launcher.ini";
	FILE										* fp					= 0;
	fopen_s(&fp, hostFileName, "rb");
	if(fp) {
		fseek(fp, 0, SEEK_END);
		int32_t										fileSize				= ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int32_t										maxRead					= (fileSize > _Size) ? _Size : fileSize;
		fread(patchhost, 1, maxRead, fp);
		fclose(fp);
		for(int32_t i=0; i<maxRead; ++i)
			if(patchhost[i] == '\r' || patchhost[i] == '\n') {
				patchhost[i] = 0;
				break;
			}
			else if(patchhost[i] == 0)
				break;
	}
	else {
		log_printf("Host file not found\n");
		return -1;
	}

	patchhost[_Size-1]						= 0;
	return 0;
}

static HWND								g_hWnd					= 0;


int32_t									downloadPatchInfoFiles	(netbuf* netConnection, const ::nwol::array_obj<::std::string>& fileNames, const ::nwol::array_pod<::SPatchDate>& fileDates, ::nwol::array_obj<::SPatchFile>& fileNamesToPatch, int32_t& lastDate)
{
	FILE* fp = 0;
	MSG msg;
	for (uint32_t iFile = 0, fileCount = (uint32_t)fileNames.size(); iFile < fileCount; ++iFile) {
		const ::std::string		& currentFileName		= fileNames[iFile];
		const ::SPatchDate		& patchDate				= fileDates[iFile];

		// process file
		FtpGet(tempFileName, currentFileName.c_str(), FTPLIB_ASCII, netConnection);
		fopen_s(&fp, tempFileName, "rb");
		if (0 == fp) {
			log_printf("Error loading patch info file: %s.\n", currentFileName.c_str());
			continue;
		}
		else
			log_printf("Processing %s...\n", currentFileName.c_str());

		fseek(fp, 0, SEEK_END);
		int32_t fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);


		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RedrawWindow(g_hWnd, 0, 0, RDW_INTERNALPAINT | RDW_INVALIDATE);

		char* buffer = (char*)malloc(fileSize);
		fread(buffer, 1, fileSize, fp);
		fclose(fp);
		::nwol::array_obj<std::string> fileNamesToPatchForThisPatch;
		getLines(buffer, fileSize, fileNamesToPatchForThisPatch);
		free(buffer);

		log_printf("Patching from info file: %s.\n", currentFileName.c_str());
		log_printf("Files for this patch:\n");
		for (uint32_t iLine = 0, lineCount = (uint32_t)fileNamesToPatchForThisPatch.size(); iLine < lineCount; ++iLine)
		{
			log_printf("- %s\n", fileNamesToPatchForThisPatch[iLine].c_str());
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			RedrawWindow(g_hWnd, 0, 0, RDW_INTERNALPAINT | RDW_INVALIDATE);
		}
	
		for(uint32_t iForThisPatch=0, fileThisPatchCount=(uint32_t)fileNamesToPatchForThisPatch.size(); iForThisPatch < fileThisPatchCount; ++iForThisPatch)
		{
			bool bFound = false;
			for(uint32_t iFileNames=0, fileNameCount=(uint32_t)fileNamesToPatch.size(); iFileNames < fileNameCount; ++iFileNames)
				if(fileNamesToPatch[iFileNames].Path == fileNamesToPatchForThisPatch[iForThisPatch])
				{
					if(fileNamesToPatch[iFileNames].PackedDate	< patchDate.PackedDate)
					{
						fileNamesToPatch[iFileNames].Year		= patchDate.Year;
						fileNamesToPatch[iFileNames].Month		= patchDate.Month;
						fileNamesToPatch[iFileNames].Day		= patchDate.Day;
						fileNamesToPatch[iFileNames].PackedDate	= patchDate.PackedDate;
					}
					bFound = true;
					break;
				}

			if(bFound)
				continue;

			SPatchFile newPatchedFile;
			newPatchedFile.Path			= fileNamesToPatchForThisPatch[iForThisPatch];
			newPatchedFile.Year			= patchDate.Year;
			newPatchedFile.Month		= patchDate.Month;
			newPatchedFile.Day			= patchDate.Day;
			newPatchedFile.PackedDate	= patchDate.PackedDate;
			fileNamesToPatch.push_back(newPatchedFile);
		}

		lastDate = lastDate > patchDate.PackedDate ? lastDate : patchDate.PackedDate;
	}
	return 0;
};


int32_t downloadPatchContents(netbuf* netConnection, SLauncherSettings& launcherSettings, const ::nwol::array_obj<SPatchFile>& fileNamesToPatch)
{
	MSG msg = {};
	FILE* fp = 0;
	for(uint32_t iFileNames=0, fileNameCount=(uint32_t)fileNamesToPatch.size(); iFileNames < fileNameCount; ++iFileNames)
	{
		const std::string&	currentFileName = fileNamesToPatch[iFileNames].Path;
		const std::string remotePath = std::string("patch/") + currentFileName;
		char remotePathFixed[300] = {};
		memcpy(remotePathFixed, remotePath.c_str(), remotePath.size());
		for (size_t iChar = 0, charCount = remotePath.size(); iChar < charCount; ++iChar)
			if (remotePathFixed[iChar] == '\\')
				remotePathFixed[iChar] = '/';

		bool bFound = false;
		for(uint32_t iForThisPatch=0, fileThisPatchCount=(uint32_t)launcherSettings.Files.size(); iForThisPatch < fileThisPatchCount; ++iForThisPatch)
			if(currentFileName == launcherSettings.Files[iForThisPatch].Path) {
				if(fileNamesToPatch[iFileNames].PackedDate > launcherSettings.Files[iForThisPatch].PackedDate) {
					FtpGet(currentFileName.c_str(), remotePathFixed, FTPLIB_BINARY, netConnection);
					fopen_s(&fp, currentFileName.c_str(), "rb");
					if(0 == fp) {
						log_printf("Error downloading patch for file: %s.\n", currentFileName.c_str());
					}
					else {
						fclose(fp);
						log_printf("File downloaded successfully: %s.\n", currentFileName.c_str());
						launcherSettings.Files[iForThisPatch] = fileNamesToPatch[iFileNames];
					}
				}
				else
					log_printf("Already up to date: %s.\n", currentFileName.c_str());
				bFound = true;

				break;
			}

		if(bFound == 1) {
			int32_t success = FtpGet(currentFileName.c_str(), remotePathFixed, FTPLIB_BINARY, netConnection);
			if(0 == success) {
				log_printf("Error downloading patch for file: %s.\n", currentFileName.c_str());
			}
			fopen_s(&fp, currentFileName.c_str(), "rb");
			if(0 == fp) {
				log_printf("Error downloading patch for file: %s.\n", currentFileName.c_str());
			}
			else {
				fclose(fp);
				log_printf("File downloaded successfully: %s.\n", currentFileName.c_str());
				launcherSettings.Files.push_back(fileNamesToPatch[iFileNames]);
			}
		}

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RedrawWindow(g_hWnd, 0, 0, RDW_INTERNALPAINT | RDW_INVALIDATE);

	}
	return 0;
}


int32_t getPatchInfoFileNamesAndDates(const ::nwol::array_obj<std::string>& fileNames, ::nwol::array_obj<std::string>& filteredFileNames, ::nwol::array_pod<SPatchDate>& fileDates)
{
	char dateStr[9] = {};
	for(uint32_t iFile = 0, fileCount = (uint32_t)fileNames.size(); iFile < fileCount; ++iFile) {
		const std::string& currentFileName = fileNames[iFile];
		if(currentFileName.size() != 13 || strcmp(&currentFileName.c_str()[9], ".txt"))
			continue;
		
		log_printf("Name: %s\n", currentFileName.c_str());
		
		memcpy(dateStr, &currentFileName.c_str()[1], 8);

		SPatchDate newDate;
		newDate.PackedDate = atoi(dateStr);
		newDate.Year	= (int16_t)	(newDate.PackedDate/10000);
		newDate.Month	= (char)	((newDate.PackedDate-newDate.Year*10000)/100);
		newDate.Day		= (char)	(newDate.PackedDate-newDate.Year*10000-newDate.Month*100);
		bool bFound = false;
		for(uint32_t i=0, dateCount = (uint32_t)fileDates.size(); i < dateCount; ++i) {
			if(fileDates[i].PackedDate > newDate.PackedDate) {
				bFound = true;
				filteredFileNames	.insert(i, currentFileName);
				fileDates			.insert(i, newDate);
				break;
			}
		}

		if(false == bFound) {
			filteredFileNames.push_back(currentFileName);
			fileDates.push_back(newDate);
		}

	}

	return 0;
}

void shutdownConsole() {
	PostMessage(GetConsoleWindow(),WM_QUIT,0,0);
	FreeConsole();
}

void initConsole(const char* windowTitle) {
	AllocConsole();
	AttachConsole(GetCurrentProcessId());

	FILE *stream=0;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	SetConsoleTitleA(windowTitle);
}

enum APPLICATION_STATE
	{	APPLICATION_STATE_INIT						= 0
	,	APPLICATION_STATE_CONNECT					= 1
	,	APPLICATION_STATE_LOAD_SETTINGS				= 2
	,	APPLICATION_STATE_PATCH_INFO_LIST			= 3
	,	APPLICATION_STATE_PATCH_INFO_DOWNLOAD		= 4
	,	APPLICATION_STATE_PATCH_CONTENT_DOWNLOAD	= 5
	,	APPLICATION_STATE_DISCONNECT				= 6
	,	APPLICATION_STATE_SAVE_SETTINGS				= 7
	,	APPLICATION_STATE_CLOSE_UI					= 8
	,	APPLICATION_STATE_INVALID					= -1
	};

static const char windowTitle		[] = "Launcher v1.0 - dreadwords@gmail.com";
static const char userName			[] = "launcher";//"launcher@warcry.com.ar";
static const char password			[] = "TheBigBang2000";
static const char backgroundName	[] = "launcher.bmp";
static const char settingsFileName	[] = "launcher.settings";
static const char installationKey	[] = "{076A6FD8-EE45-4A83-B3C9-C7C34E7CAFDD}";
static const char CLASS_NAME		[]  = "LauncherWindow";
static HINSTANCE g_hInstance = 0;
static char patchhost[256] = {};

static const uint32_t WIDTH		= 480;
static const uint32_t HEIGHT	= 300;

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT val = 0;
	static HBITMAP hBitmap = 0;
    PAINTSTRUCT     ps;
    HDC             hdc;
    BITMAP          bitmap;
    HDC             hdcMem;
    HGDIOBJ         oldBitmap;

	std::string lastLogLine = "Initializing launcher";
	if( logLines.size() )
		lastLogLine = logLines[logLines.size()-1];
	int32_t logLineLen = (int32_t)lastLogLine.size();
	switch(uMsg)
	{
	case WM_CREATE:
		if(0 == hBitmap)
			hBitmap = (HBITMAP)LoadImage(g_hInstance, backgroundName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		val = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
    case WM_PAINT:

        hdc = BeginPaint(hWnd, &ps);
        hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hBitmap);
		
        GetObject(hBitmap, sizeof(BITMAP), &bitmap);
        //BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
		BitBlt(hdc, 0, 0, WIDTH, HEIGHT, hdcMem, 0, 0, SRCCOPY);
		
        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

		if( logLineLen )
			TextOut(hdc, 1, HEIGHT -22, lastLogLine.c_str(), logLineLen);//lastLogLine.size()-1); 
        EndPaint(hWnd, &ps);
        break;
	case WM_CLOSE:
		if(hWnd == g_hWnd)
			DestroyWindow(hWnd);
		else
			val = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		if(hWnd == g_hWnd)
			PostQuitMessage(0);
		else
			val = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	default:
		val = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}
	return val;
};
HWND createWindow(HINSTANCE hInstance)
{
    WNDCLASS wc = { };

    wc.lpfnWndProc   = windowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
	wc.style = CS_HREDRAW | CS_VREDRAW;  

    // Register the window class.
    RegisterClass(&wc);

    // Create the window.
	int32_t Width, Height, X, Y;

	Width	= WIDTH;
	Height	= HEIGHT;
	X		= (GetSystemMetrics(SM_CXSCREEN)>>1)-(Width		>>1);
	Y		= (GetSystemMetrics(SM_CYSCREEN)>>1)-(Height	>>1);

    g_hWnd = CreateWindowEx(
        CS_HREDRAW | CS_VREDRAW,								// Optional window styles.
        CLASS_NAME,						// Window class
        windowTitle,					// Window text
        (WS_POPUP//WS_OVERLAPPED     
        //|WS_CAPTION        
        //|WS_SYSMENU        
        //|WS_THICKFRAME     
        //|WS_MINIMIZEBOX    
        //|WS_MAXIMIZEBOX
		),//WS_OVERLAPPEDWINDOW,			// Window style

        // Size and position
        X, Y, Width, Height,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (g_hWnd == NULL)
    {
        return 0;
    }

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd); 
	return g_hWnd;
}

APPLICATION_STATE executeStep(APPLICATION_STATE applicationState, HINSTANCE hInstance)
{
	static netbuf* netConnection = 0;
	static SLauncherSettings launcherSettings = {};
	static ::nwol::array_obj<std::string> fileNames, filteredFileNames;
	static ::nwol::array_pod<SPatchDate> fileDates;
	static ::nwol::array_obj<SPatchFile> fileNamesToPatch;
	int32_t success = 0;
	switch(applicationState)
	{
	case APPLICATION_STATE_INIT				:
		FtpInit();

		initConsole(windowTitle);
		createWindow(hInstance);
		if(loadHost(patchhost))
			return APPLICATION_STATE_INVALID;

		break;
	case APPLICATION_STATE_CONNECT			:	
		netConnection = 0;
		success = FtpConnect(patchhost, &netConnection);
		if (0 == success) {
			log_printf("Failed to connect to host: %s.\n", patchhost);
			FtpQuit(netConnection);
			return APPLICATION_STATE_INVALID;
		}

		success = FtpLogin(userName, password, netConnection);

		if(0 == success) {
			log_printf("Failed to authenticate user: %s.\n", userName);
			FtpQuit(netConnection);
			return APPLICATION_STATE_INVALID;
		}

		break;

	case APPLICATION_STATE_LOAD_SETTINGS	:	
		loadSettings(launcherSettings, settingsFileName);

		if( 0 > listRemote(netConnection, fileNames) ) {
			log_printf("Error downloading patch history from server.\n");
			return APPLICATION_STATE_INVALID;
		}
		break;

	case APPLICATION_STATE_PATCH_INFO_LIST	:
		getPatchInfoFileNamesAndDates(fileNames, filteredFileNames, fileDates);
		break;

	case APPLICATION_STATE_PATCH_INFO_DOWNLOAD:
		downloadPatchInfoFiles(netConnection, filteredFileNames, fileDates, fileNamesToPatch, launcherSettings.LastDate);
		break;

	case APPLICATION_STATE_PATCH_CONTENT_DOWNLOAD:
		downloadPatchContents(netConnection, launcherSettings, fileNamesToPatch);
		break;

	case APPLICATION_STATE_DISCONNECT:
		FtpQuit(netConnection);
		break;

	case APPLICATION_STATE_SAVE_SETTINGS:
		if(0 > saveSettings(launcherSettings, settingsFileName))
			log_printf("Failed to save patch state.\n");
		break;

	case APPLICATION_STATE_CLOSE_UI:
		shutdownConsole();
		PostMessage(g_hWnd, WM_CLOSE, 0, 0);
		break;

	default:
		return APPLICATION_STATE_INVALID;
		break;
	}
	return (APPLICATION_STATE)(applicationState+1);
}

int CALLBACK WinMain
	( _In_ HINSTANCE hInstance
	, _In_ HINSTANCE /* hPrevInstance	*/
	, _In_ LPSTR     /* lpCmdLine		*/
	, _In_ int       /* nCmdShow		*/
	)
{
#if defined(NWOL_DEBUG_ENABLED)
	int tmp = _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_DELAY_FREE_MEM_DF);
	// Clear the upper 16 bits and OR in the desired freqency
	tmp = (tmp & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_16_DF;

	// Set the new bits
	_CrtSetDbgFlag(tmp);
#endif
	g_hInstance = hInstance;

	APPLICATION_STATE appState = APPLICATION_STATE_INIT;
	MSG msg = { };
	appState = executeStep(appState, hInstance);

	//
	while(msg.message != WM_QUIT)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
 		RedrawWindow(g_hWnd, 0, 0, RDW_INTERNALPAINT | RDW_INVALIDATE);
		appState = executeStep(appState, hInstance);
	}

	UnregisterClass(CLASS_NAME, hInstance);


	STARTUPINFOA startupInfo = {};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInformation = {};
	char processName[] = "\"system\\L2.exe\"";
	if( !CreateProcessA
	( 0							//_In_opt_    LPCTSTR              	lpApplicationName,
	, processName				//_Inout_opt_ LPTSTR               	lpCommandLine,
	, 0							//_In_opt_    LPSECURITY_ATTRIBUTES	lpProcessAttributes,
	, 0							//_In_opt_    LPSECURITY_ATTRIBUTES	lpThreadAttributes,
	, false						//_In_        BOOL                 	bInheritHandles,
	, 0							//_In_        DWORD                	dwCreationFlags,
	, 0							//_In_opt_    LPVOID               	lpEnvironment,
	, 0							//_In_opt_    LPCTSTR              	lpCurrentDirectory,
	, &startupInfo				//_In_        LPSTARTUPINFO        	lpStartupInfo,
	, &processInformation		//_Out_       LPPROCESS_INFORMATION	lpProcessInformation
	) )
    {
		log_printf( "Game executable failed to load (%d).\n", GetLastError() );
		return 0;
    }

    // Wait until child process exits.
    WaitForSingleObject( processInformation.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( processInformation.hProcess );
    CloseHandle( processInformation.hThread );

	return 0;
}