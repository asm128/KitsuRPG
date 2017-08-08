#include "kitsunwo.h"

#include "nwol_text.h"
#include "ascii_screen.h"
#include "gui.h"

#include "nwol_runtime_impl.h"

#include <thread>
#include <process.h>

DEFINE_RUNTIME_INTERFACE_FUNCTIONS(SApplication, "Vulgar Display of Power", 0, 1);

#define errored(functionCall) (::nwol::failed(functionCall))

#define errmsg(functionCall, format, ...)																				\
	{																													\
		::nwol::error_t _err_errMy = 0;																					\
		if errored(_err_errMy = functionCall) {	error_printf(" Error code: 0x%x. " format, _err_errMy, __VA_ARGS__); }	\
	}

int32_t cleanup	(::SApplication& instanceApp) { 
	::nwol::shutdownASCIIScreen();

	nwol_necall(::networkDisable(instanceApp), "Network not enabled?");
	return 0; 
}

int32_t setup (::SApplication& instanceApp)
{ 
	::nwol::SGUI							& guiSystem					= instanceApp.GUI;

	::nwol::initASCIIScreen(guiSystem.TargetSizeASCII.x, guiSystem.TargetSizeASCII.y);
	char															moduleTitle[240]						= {};
	uint8_t															moduleTitleLen							= (uint8_t)::nwol::size(moduleTitle);
	nwol_necall(::nwol_moduleTitle(moduleTitle, &moduleTitleLen), "If this fails then something weird is going on.");
	::nwol::setASCIIScreenTitle(moduleTitle);

	::klib::initGame(instanceApp.Game);

	errmsg(::networkEnable(instanceApp), "Failed to enable network.");

	guiSystem.TargetSizeASCII.x			= nwol::getASCIIBackBufferWidth		();
	guiSystem.TargetSizeASCII.y			= nwol::getASCIIBackBufferHeight	();

	static const ::nwol::glabel				newControlLabel				= "Exit";
	::nwol::SGUIControl						newControl;

	newControl.AreaASCII				= {1, 1, (int32_t)newControlLabel.size(), 1}	;
	newControl.Text						= newControlLabel								;
	nwol_necall(::nwol::createControl(guiSystem, newControl), "Failed to create control: %s.", newControl.Text.c_str());
	return 0; 
}

#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif


int32_t update(::SApplication& instanceApp, bool exitRequested)
{
	if(exitRequested)
		return ::nwol::APPLICATION_STATE_EXIT;

	::nwol::SInput								& inputSystem			= instanceApp.Input;
	::nwol::pollInput(inputSystem);
	//if(inputSystem.Keys[VK_ESCAPE])
	//	return ::nwol::APPLICATION_STATE_EXIT;

	::nwol::SGUI								& guiSystem				= instanceApp.GUI;
	::nwol::updateGUI(guiSystem, inputSystem);

	::nwol::array_pod<::nwol::CONTROL_FLAG>		& controlFlags			= guiSystem.Controls.ControlFlags;
	for(uint32_t iControl = 0, controlCount = controlFlags.size(); iControl < controlCount; ++iControl)
		if(::nwol::bit_true(controlFlags[iControl], ::nwol::CONTROL_FLAG_EXECUTE)) {
			info_printf("Execute control %u.", iControl);
			switch(iControl)
			{
			case 0:		
				return ::nwol::APPLICATION_STATE_EXIT; 
				break;
			default:
				break;
			}
		}
	return 0; 
}

int32_t render(::SApplication& instanceApp) { 
	::nwol::clearASCIIBackBuffer(' ', COLOR_WHITE);

	::klib::SGame							& instanceGame		= instanceApp.Game;
	::nwol::SApplicationNetworkClient		& appNetwork		= instanceApp.NetworkClient;
	
	{	// here we update the game instance with the data received from the server.
		::nwol::CLock							thelock				(appNetwork.ServerTimeMutex);
		instanceGame.ServerTime				= appNetwork.ServerTime;
	}
	instanceGame.FrameInput				= instanceApp.Input;
	::klib::drawAndPresentGame(instanceGame);

	::nwol::SASCIITarget					target;
	::nwol::error_t							errMy			= ::nwol::getASCIIBackBuffer(target);
	
	::nwol::renderGUIASCII(target, instanceApp.GUI);

	::nwol::presentASCIIBackBuffer();
	
	return 0; 
}
