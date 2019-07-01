#include "kitsunwo.h"

#include "klib_ascii_screen.h"
#include "gui.h"

#include "nwol_runtime_impl.h"

#include <thread>
#include <process.h>

DEFINE_RUNTIME_INTERFACE_FUNCTIONS(SApplication, "Vulgar Display of Power", 0, 1);

#define errored(functionCall) (::nwol::failed(functionCall))

#define errmsg(functionCall, format, ...)																									\
	{																																		\
		::gpk::error_t													_err_errMy								= 0;						\
		gerror_if(errored(_err_errMy = functionCall), "'%s' failed. Error code: 0x%x. " format, #functionCall, _err_errMy, __VA_ARGS__);		\
	}

int32_t														cleanup									(::SApplication& instanceApp)										{
	::klib::shutdownASCIIScreen();
	gpk_necall(::networkDisable(instanceApp), "Network not enabled?");
	return 0; 
}

int32_t														setup									(::SApplication& instanceApp)										{ 
	::nwol::SGUI													& guiSystem								= instanceApp.GUI;

	::klib::initASCIIScreen(guiSystem.TargetSizeASCII.x, guiSystem.TargetSizeASCII.y);
	char															moduleTitle[240]						= {};
	uint8_t															moduleTitleLen							= (uint8_t)::nwol::size(moduleTitle);
	gpk_necall(::nwol_moduleTitle(moduleTitle, &moduleTitleLen), "If this fails then something weird is going on.");
	::klib::setASCIIScreenTitle(moduleTitle);

	::klib::initGame(instanceApp.Game);

	errmsg(::networkEnable(instanceApp), "Failed to enable network.");

	::klib::SASCIITarget											asciiTarget								= {};
	::klib::getASCIIBackBuffer(asciiTarget);
	guiSystem.TargetSizeASCII.x									= asciiTarget.Width		();
	guiSystem.TargetSizeASCII.y									= asciiTarget.Height	();

	static const ::nwol::glabel										newControlLabel							= "Exit";
	::nwol::SGUIControl												newControl								= {};

	newControl.AreaASCII										= {1, 1, (int32_t)newControlLabel.size(), 1}	;
	newControl.Text												= newControlLabel								;
	gpk_necall(::nwol::createControl(guiSystem, newControl), "Failed to create control: %s.", newControl.Text.c_str());
	return 0; 
}

#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif

int32_t														update									(::SApplication& instanceApp, bool exitRequested)					{
	if(exitRequested)
		return ::nwol::APPLICATION_STATE_EXIT;

	::nwol::SInput													& inputSystem							= instanceApp.Input;
	gerror_if(errored(::nwol::pollInput(inputSystem)), "Some weird thing happened which prevented pollInput() to succeed. I have no idea why this would ever happen.");
	//if(inputSystem.Keys[VK_ESCAPE])
	//	return ::nwol::APPLICATION_STATE_EXIT;

	::nwol::SGUI													& guiSystem								= instanceApp.GUI;
	gerror_if(errored(::nwol::updateGUI(guiSystem, inputSystem)), "Why would we fail to update the GUI?");

	::nwol::array_pod<::nwol::CONTROL_STATE>						& controlFlags							= guiSystem.Controls.ControlFlags;
	for(uint32_t iControl = 0, controlCount = controlFlags.size(); iControl < controlCount; ++iControl)
		if(::nwol::bit_true(controlFlags[iControl], ::nwol::CONTROL_STATE_EXECUTE)) {
			info_printf("Execute control %u.", iControl);
			switch(iControl) {
			case 0	: return ::nwol::APPLICATION_STATE_EXIT; 
			default	:
				break;
			}
		}
	return 0; 
}

int32_t														render									(::SApplication& instanceApp)										{ 
	::klib::clearASCIIBackBuffer(' ', COLOR_WHITE);

	::klib::SGame													& instanceGame							= instanceApp.Game;
	::gddm::SFrameworkNetworkClient									& appNetwork							= instanceApp.NetworkClient;
	{	// here we update the game instance with the data received from the server.
		::nwol::CMutexGuard												thelock				(appNetwork.ServerTimeMutex);
		instanceGame.ServerTime										= appNetwork.ServerTime;
	}
	memcpy(instanceGame.FrameInput.Mouse.Buttons, instanceApp.Input.Mouse.Buttons, sizeof(instanceApp.Input.Mouse.Buttons));
	instanceGame.FrameInput.Mouse.Deltas							= {instanceApp.Input.Mouse.Deltas.x, instanceApp.Input.Mouse.Deltas.y};
	memcpy(instanceGame.FrameInput.Keys			, instanceApp.Input.Keys		, sizeof(instanceApp.Input.Keys			));
	memcpy(instanceGame.FrameInput.PreviousKeys	, instanceApp.Input.PreviousKeys, sizeof(instanceApp.Input.PreviousKeys	));
	::klib::SASCIITarget							target;
	::klib::getASCIIBackBuffer						(target);
	::klib::drawAndPresentGame(instanceGame, target);
	//::nwol::renderGUIASCII(target, instanceApp.GUI);
	::klib::presentASCIIBackBuffer();
	return 0; 
}
