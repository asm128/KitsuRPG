#define NETLIB_SERVER_MODE_CLIENT
#include "draw.h"
#include "netlib_node.h"
#include "netlib_command.h"

#include <time.h>
#include <process.h>

#include <crtdbg.h>

// Use this function to draw our game data
void draw( klib::SGame& instanceGame ) // 
{
	::nwol::clearASCIIBackBuffer(' ', COLOR_WHITE);
 	drawAndPresentGame(instanceGame);
	::nwol::presentASCIIBackBuffer();
};

bool bAreCommsRunningInThisDamnStuffCode = false;

::nwol::error_t								nodeHandleUserData						(::gdnet::SNetworkNode* /*client*/, const char* /*buffer*/, uint32_t /*bufferLen*/)		{ return 0; }
int runCommunications(klib::SGame& instanceGame);
void runCommunications(void* pInstanceGame)
{
	if(0 == pInstanceGame)
		return;

	klib::SGame& instanceGame = *(klib::SGame*)pInstanceGame;

	runCommunications(instanceGame);

	for(uint32_t i=0; i<10; ++i)
		if(instanceGame.Flags & klib::GAME_FLAGS_RUNNING)
			Sleep(1000);

	bAreCommsRunningInThisDamnStuffCode = false;
}

int main(int argc, char **argv)
{
#if defined(NWOL_DEBUG_ENABLED)
	int tmp = _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_DELAY_FREE_MEM_DF);
	// Clear the upper 16 bits and OR in the desired freqency
	tmp = (tmp & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_16_DF;
	// Set the new bits
	_CrtSetDbgFlag(tmp);
#endif

	::nwol::initASCIIScreen(klib::SGlobalDisplay::Width, klib::SGlobalDisplay::Depth);
	
	if( ::gdnet::endpointSystemInitialize() )
	{
		error_printf("Failed to initialize network.");
		return -1;
	}

	klib::SGame* pInstancedGame = new klib::SGame;
	klib::SGame& instanceGame = *pInstancedGame;

	klib::initGame(instanceGame);

	bAreCommsRunningInThisDamnStuffCode = true;
	_beginthread(runCommunications, 0, pInstancedGame);
	Sleep(1000);


	while(instanceGame.Flags & klib::GAME_FLAGS_RUNNING)
	{
		if(false == bAreCommsRunningInThisDamnStuffCode) {	// if this is false it means the comms thread died 
			bAreCommsRunningInThisDamnStuffCode = true;
			_beginthread(runCommunications, 0, pInstancedGame);
		}

		pollInput(instanceGame.FrameInput);
		draw(instanceGame);
	}

	while(bAreCommsRunningInThisDamnStuffCode)
		Sleep(500);

	if(pInstancedGame)
		delete(pInstancedGame);

	::nwol::shutdownASCIIScreen();
	::gdnet::endpointSystemShutdown();

	return 0;
}

int runCommunications(klib::SGame& instanceGame)
{
	::gdnet::SNetworkNode	instanceClient;
	nwol_necall(::gdnet::nodeInit(instanceClient, { {}, {{192, 168, 1, 27}, 45678} }, ::gdnet::TRANSPORT_PROTOCOL_UDP), "Failed to connect to server.");
	nwol_necall(::gdnet::nodeConnect(instanceClient), "Failed to connect to server.");

	::nwol::error_t result = 0;
	while(nwol::bit_true(instanceGame.Flags, klib::GAME_FLAGS_RUNNING)) {
		if(false == ::gdnet::ping(instanceClient.Endpoints)) {	// Ping before anything else to make sure everything is more or less in order.
			error_print("Ping timeout.");
			result = -1; 
			break;
		}

		// get server time
		uint64_t current_time;
		be_if(errored(result = ::gdnet::time(instanceClient.Endpoints, current_time)), "Failed to get server time.");
		
		{	// here we update the game instance with the data received from the server.
			::nwol::CMutexGuard thelock(instanceGame.ServerTimeMutex);
			instanceGame.ServerTime = current_time;
			info_printf("Client instance updated successfully.");
		}

		// Disconnect if the game was closed.
		if(nwol::bit_false(instanceGame.Flags, klib::GAME_FLAGS_RUNNING))
			break;

		Sleep(1000);
	}

	::gdnet::nodeRequestDisconnect(instanceClient);
	bAreCommsRunningInThisDamnStuffCode		= false; 
	gbit_clear(instanceGame.Flags, klib::GAME_FLAGS_CONNECTED);
	instanceClient.Endpoints				= {};

	return result;
}

int WINAPI WinMain 
	(	_In_		HINSTANCE	// hInstance
	,	_In_opt_	HINSTANCE	// hPrevInstance
	,	_In_		LPSTR		// lpCmdLine
	,	_In_		int			// nShowCmd
	)
{
	if(0 > main(__argc, __argv))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
};