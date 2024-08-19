#pragma once

#include "modem/AppModem.hpp"

/// @brief Main state machine class
class AppMainStateMachine
{

public:
	bool Init(void);

	bool Process(void);

private:
	enum class States {
		INITIALIZED,		// Initialized
		WAIT_FOR_GNSS,		// Waiting for GNSS fix
		WAIT_FOR_NETWORK,	// Waitign for IP
		NORMAL_OPERATION,	// Normal operation
		GO_TO_SLEEP,		// Go to power saving mode
		WAKE_FROM_SLEEP,	// Wake from power saving
		ERROR,				// Any failure
	};

	enum class NetworkStates {
		INITIALIZED,	// The modem is initialized properly
		ATTACHED,		// A network is attached
		CONFIGURED,		// The network has been configured
		APP_ENABLED,	// The IP network is enabled
		APP_HAS_IP,		// An IP has been assigned to the modem
		ERROR,			// Any failure
	};
	bool ProcessNetwork(void);

	bool CheckIp(void);
	bool SendPoint(void);

	AppModem app_modem;

	NetworkStates 	current_network_state{NetworkStates::ERROR};
	NetworkStates 	previous_network_state{NetworkStates::ERROR};
	States			current_main_state{States::ERROR};

	GpsInfo 		last_gps_info;
	char 			self_ip[17];

};
