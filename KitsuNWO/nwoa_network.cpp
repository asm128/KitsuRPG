#include "kitsunwo.h"
#include "netlib_command.h"
#include "nwol_sleep.h"

#include <thread>

#include <process.h>

::nwol::error_t								nodeHandleUserData						(::gdnet::SNetworkNode* /*client*/, const char* /*buffer*/, uint32_t /*bufferLen*/)		{ return 0; }
::nwol::error_t								runCommunications						(::gddm::SFrameworkNetworkClient& appNetwork)											{
	::gdnet::SNetworkNode							& instanceClient						= appNetwork.Connection;
	nwol_necall(::gdnet::nodeInit		(instanceClient, instanceClient.Address, ::gdnet::TRANSPORT_PROTOCOL_UDP), "%s", "Failed to initialize client connection.");
	nwol_necall(::gdnet::nodeConnect	(instanceClient), "%s", "Failed to connect.");

	::nwol::error_t									result									= 0;
	while gbit_true(appNetwork.State, ::gddm::NETWORK_STATE_ENABLED) {
		break_error_if(result = ::gdnet::ping(instanceClient.Endpoints) ? 0 : -1, "%s", "Ping timeout.");	// Ping before anything else to make sure everything is more or less in order.
		::nwol::sleep(500);

		uint64_t										current_time;
		break_error_if(errored(result = ::gdnet::time(instanceClient.Endpoints, current_time)), "Failed to get server time.");		// get server time
		{	// here we update the game instance with the data received from the server.
			::nwol::CMutexGuard								thelock								(appNetwork.ServerTimeMutex);
			appNetwork.ServerTime						= current_time;
			info_printf("%s", "Client instance updated successfully.");
			char											timestring		[256]				= {};
			time_t											timeWithUnreliableSize				= (time_t)current_time;
	#if defined(__WINDOWS__)
			::ctime_s(timestring, sizeof(char)*256, &timeWithUnreliableSize);
	#else
			strcpy(timestring, ctime(&curTimeWithUnreliableSize));
	#endif
			timestring[::strlen(timestring)-1]			= 0;
			info_printf("Received time (%llu) (%s) from %u.%u.%u.%u:%u.", current_time, timestring, NWOL_EXPAND_IPV4_UINT32(instanceClient.Address.Remote));

		}
		break_info_if(gbit_false(appNetwork.State, ::gddm::NETWORK_STATE_ENABLED), "Disconnect as the network was disabled.");
		::nwol::sleep(500);
	}
	::gdnet::nodeRequestDisconnect(instanceClient);
	gbit_clear(appNetwork.State, ::gddm::NETWORK_STATE_RUNNING);
	instanceClient.Endpoints					= {};
	return result;
}

void										runCommunications						(void* pInstanceApp)								{
	info_printf("Communications loop initializing.");
	ret_error_if(0 == pInstanceApp, "Invalid argument: SApplication* input parameter is null.")
	::SApplication									& instanceApp							= *(::SApplication*)pInstanceApp;
	::gddm::SFrameworkNetworkClient					& instanceAppNetwork					= instanceApp.NetworkClient;
	const ::nwol::error_t							errMyComm								= ::runCommunications(instanceAppNetwork);
	gbit_clear(instanceAppNetwork.State, ::gddm::NETWORK_STATE_RUNNING);			
	info_printf("Communications loop exited with code 0x%X.", errMyComm);
}

::nwol::error_t								networkEnable							(::SApplication& instanceApp)						{
	nwol_necall(::gdnet::endpointSystemInitialize(), "Failed to initialize network.");
	info_printf("%s", "Network successfully initialized.");

	::gddm::SFrameworkNetworkClient					& instanceAppNetwork					= instanceApp.NetworkClient;
	gbit_set(instanceAppNetwork.State, ::gddm::NETWORK_STATE_ENABLED | ::gddm::NETWORK_STATE_RUNNING);
	::_beginthread(::runCommunications, 0, &instanceApp);
	::nwol::sleep(1000);
	info_printf("%s", "Communications thread started.");
	return 0;
}

::nwol::error_t								networkDisable							(::SApplication& instanceApp)						{
	::gddm::SFrameworkNetworkClient					& instanceAppNetwork					= instanceApp.NetworkClient;
	gbit_clear(instanceAppNetwork.State, ::gddm::NETWORK_STATE_ENABLED);

	uint32_t										waitLimit								= 500;
	uint32_t										waitTime								= 10;
	while(gbit_true(instanceAppNetwork.State, ::gddm::NETWORK_STATE_RUNNING) && (waitLimit -= waitTime) > 0)
		::nwol::sleep(waitTime);

	instanceAppNetwork.Connection.Endpoints		= {};
	
	::nwol::sleep(250);
	::gdnet::endpointSystemShutdown();
	return 0;
}
