#include "AppMainStateMachine.hpp"
#include "AppCore.hpp"

#include "protobuffers/PayloadFormatter.hpp"
#include "utils/Base64.hpp"

#include "log.h"

bool AppMainStateMachine::Init(void)
{
    if (app_modem.Init())
	{
		if (   app_modem.EnableGnss(true)
			&& app_modem.SetPhoneMode(true)
			&& app_modem.CheckSimReady()
			&& app_modem.ConfigureLte())
		{
			current_network_state = NetworkStates::INITIALIZED;
            log_debug("Modem init success");
		}
        else
        {
        	log_warn("Modem init failure");
        }
	}

    current_main_state = States::INITIALIZED;

    return (current_network_state == NetworkStates::INITIALIZED);
}

bool AppMainStateMachine::Process(void)
{
    // Get GPS info
    memset(&last_gps_info, 0, sizeof(GpsInfo));
    app_modem.RetrieveGnssData(last_gps_info);

    // GPS fix lost
    if (current_main_state > States::WAIT_FOR_GNSS
        && !last_gps_info.GetFix())
    {
        current_main_state = States::WAIT_FOR_GNSS;
    }

    ProcessNetwork();

    // Network lost
    if (current_main_state > States::WAIT_FOR_NETWORK
        && current_network_state != NetworkStates::APP_HAS_IP)
    {
        current_main_state = States::WAIT_FOR_NETWORK;
    }

    switch (current_main_state)
    {
        case States::INITIALIZED:
        {
            //log_trace("States::INITIALIZED");

            current_main_state = States::WAIT_FOR_GNSS;

            break;
        }
        case States::WAIT_FOR_GNSS:
        {
            //log_trace("States::WAIT_FOR_GNSS");

            if (last_gps_info.GetFix())
            {
                current_main_state = States::WAIT_FOR_NETWORK;
            }

            break;
        }
        case States::WAIT_FOR_NETWORK:
        {
            //log_trace("States::WAIT_FOR_NETWORK");

            if (current_network_state == NetworkStates::APP_HAS_IP)
            {
                current_main_state = States::NORMAL_OPERATION;
            }

            // Store point in EEPROM

            break;
        }
        case States::NORMAL_OPERATION:
        {
            //log_trace("States::NORMAL_OPERATION");

            if (SendPoint())
            {
                log_info("Point sent to the API");
                AppCore_BlockingDelayMs(30e3);
            }

            break;
        }
        case States::GO_TO_SLEEP:
        {
            //log_trace("States::GO_TO_SLEEP");
            break;
        }
        case States::WAKE_FROM_SLEEP:
        {
            //log_trace("States::WAKE_FROM_SLEEP");
            break;
        }
        default:
        {
            log_error("States::ERROR state reached !");
            current_main_state = States::ERROR;
            break;
        }
    }

    return (current_main_state != States::ERROR);
}

bool AppMainStateMachine::ProcessNetwork(void)
{
    if (current_network_state == NetworkStates::ERROR)
    {
        return false;
    }

    // If a network was attached but we lost it, go back to start
    if (current_network_state > NetworkStates::CONFIGURED
        && !app_modem.CheckNetworkRegistration())
    {
    	// We should not fall here after being configured
        current_network_state = NetworkStates::INITIALIZED;
    }

    switch (current_network_state)
    {
        case NetworkStates::INITIALIZED:
        {
            //log_trace("NetworkStates::INITIALIZED");

            if (app_modem.CheckNetworkRegistration())
            {
                current_network_state = NetworkStates::ATTACHED;
            }
            break;
        }
        case NetworkStates::ATTACHED:
        {
            //log_trace("NetworkStates::ATTACHED");

            app_modem.EnableGprs(false, QUOTE(""), QUOTE(""), QUOTE(""));
            if (app_modem.EnableGprs(true, QUOTE("iot.1nce.net"), QUOTE(""), QUOTE("")))
            {
                current_network_state = NetworkStates::CONFIGURED;
            }
            break;
        }
        case NetworkStates::CONFIGURED:
        {
            //log_trace("NetworkStates::CONFIGURED");

            if (app_modem.EnableAppNetwork(true))
            {
                current_network_state = NetworkStates::APP_ENABLED;
            }            
            break;
        }
        case NetworkStates::APP_ENABLED:
        {
            //log_trace("NetworkStates::APP_ENABLED");

            if (CheckIp())
            {
                log_info("IP address assigned %s", self_ip);
                current_network_state = NetworkStates::APP_HAS_IP;
            }
            
            break;
        }
        case NetworkStates::APP_HAS_IP:
        {
            //log_trace("NetworkStates::APP_HAS_IP");

            // IP lost, go back to start
            if (!CheckIp())
            {
                current_network_state = NetworkStates::INITIALIZED;
            }

            break;
        }
        default:
        {
            log_error("NetworkStates::ERROR state reached !");
            current_network_state = NetworkStates::ERROR;
            break;
        }
    }

    return (current_network_state != NetworkStates::ERROR);
}

bool AppMainStateMachine::CheckIp(void)
{
    if (app_modem.GetIp(self_ip))
    {
        if (strcmp(self_ip, "0.0.0.0") != 0)
        {
            return true;
        }
    }

    return false;
}

bool AppMainStateMachine::SendPoint(void)
{
    uint8_t payload[50];
    size_t  payload_size = PayloadFormatter_MakePayload(last_gps_info, 0xFF, 0, payload, sizeof(payload));
    uint8_t base64_payload[255]{0};
    size_t  base64_payload_size = 0;

    const char* API_URL = QUOTE("coap://coap.os.1nce.com:5683");

    if (payload != nullptr)
    {
        // Quote at beginning of payload
        base64_payload[0] = '"';
        if (EncodeBase64(base64_payload+1, sizeof(base64_payload), payload, payload_size, &base64_payload_size))
        {
            // Quote at end of payload
            base64_payload[1+base64_payload_size] = '"';
            return app_modem.SendCoapRequest(API_URL, AppModem::ReqMethod::POST, QUOTE(""), QUOTE(""), reinterpret_cast<char*>(base64_payload));
        }
    }

    return false;
}
