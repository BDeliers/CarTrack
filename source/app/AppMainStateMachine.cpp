#include "AppMainStateMachine.hpp"
#include "AppCore.hpp"

#include "protobuffers/PayloadFormatter.hpp"
#include "utils/Base64.hpp"

#include <math.h>

#include "log.h"

bool AppMainStateMachine::Init(AppDebug& app_debug)
{
    this->app_debug = &app_debug;

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
    GpsInfo tmp_gps_info;

    if (current_main_state != States::WAKE_FROM_SLEEP)
    {
        // Get GPS info
        memset(&tmp_gps_info, 0, sizeof(GpsInfo));
        app_modem.RetrieveGnssData(tmp_gps_info);

        // GPS fix lost
        if (current_main_state > States::WAIT_FOR_GNSS
            && !tmp_gps_info.GetFix())
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
    }

    switch (current_main_state)
    {
        case States::INITIALIZED:
        {
            current_main_state = States::WAIT_FOR_GNSS;

            break;
        }
        case States::WAIT_FOR_GNSS:
        {
            if (tmp_gps_info.GetFix())
            {
                current_main_state = States::WAIT_FOR_NETWORK;
            }
            else
            {
            	AppCore_BlockingDelayMs(2e3);
            }

            break;
        }
        case States::WAIT_FOR_NETWORK:
        {
            if (current_network_state == NetworkStates::APP_HAS_IP)
            {
                current_main_state = States::NORMAL_OPERATION;
            }

            // Store point in EEPROM

            break;
        }
        case States::NORMAL_OPERATION:
        {
            // Check that we really moved
            if (FlatDistanceBetweenPoints(tmp_gps_info, current_gps_info) >= GPS_DISTANCE_THRESHOLD)
            {
                // Store previous GPS position
                memcpy(&previous_gps_info, &current_gps_info, sizeof(GpsInfo));
                // Update current GPS position
                memcpy(&current_gps_info, &tmp_gps_info, sizeof(GpsInfo));

                if (SendPoint())
                {
                    log_info("Point sent to the API");
                }
            }
            else
            {
                log_info("Point discarded");
            }

            // Enter deep sleep
            current_main_state = States::GO_TO_SLEEP;

            break;
        }
        case States::GO_TO_SLEEP:
        {
            log_info("Go to sleep");
            current_main_state = States::WAKE_FROM_SLEEP;

            app_debug->Flush();

            AppCore_DeepSleepMs(60e3);
            //AppCore_BlockingDelayMs(60e3);
            //current_main_state = States::NORMAL_OPERATION;
            break;
        }
        case States::WAKE_FROM_SLEEP:
        {
            log_info("Back from sleep");
            current_main_state = States::INITIALIZED;
            
            app_debug->Flush();
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
    if (current_network_state > NetworkStates::INITIALIZED
        && !app_modem.CheckNetworkRegistration())
    {
    	// We should not fall here after being configured
        current_network_state = NetworkStates::INITIALIZED;

        log_trace("Network lost");
    }

    switch (current_network_state)
    {
        case NetworkStates::INITIALIZED:
        {
            if (app_modem.CheckNetworkRegistration())
            {
                current_network_state = NetworkStates::ATTACHED;
            }
            break;
        }
        case NetworkStates::ATTACHED:
        {
            app_modem.EnableGprs(false, QUOTE(""), QUOTE(""), QUOTE(""));
            if (app_modem.EnableGprs(true, QUOTE("iot.1nce.net"), QUOTE(""), QUOTE("")))
            {
                current_network_state = NetworkStates::CONFIGURED;
            }
            break;
        }
        case NetworkStates::CONFIGURED:
        {
            if (app_modem.EnableAppNetwork(true))
            {
                current_network_state = NetworkStates::APP_ENABLED;
            }            
            break;
        }
        case NetworkStates::APP_ENABLED:
        {
            if (CheckIp())
            {
                log_info("IP address assigned %s", self_ip);
                current_network_state = NetworkStates::APP_HAS_IP;
            }
            
            break;
        }
        case NetworkStates::APP_HAS_IP:
        {
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
    size_t  payload_size = PayloadFormatter_MakePayload(current_gps_info, 0xFF, 0, payload, sizeof(payload));
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

uint32_t AppMainStateMachine::FlatDistanceBetweenPoints(const GpsInfo& point1, const GpsInfo& point2)
{
    const float RadiusEarth = 6371000.0;    // In meters

    int32_t lat1 = point1.GetLatitude();
    int32_t lon1 = point1.GetLongitude();
    int32_t lat2 = point2.GetLatitude();
    int32_t lon2 = point2.GetLongitude();

    float cos_lat1 = cos(lat1 * M_PI / 180000000);
    float sin_lat1 = sin(lat1 * M_PI / 180000000);
    float cos_lat2 = cos(lat2 * M_PI / 180000000);
    float sin_lat2 = sin(lat2 * M_PI / 180000000);
    float cos_lon1 = cos(lon1 * M_PI / 180000000);
    float sin_lon1 = sin(lon1 * M_PI / 180000000);
    float cos_lon2 = cos(lon2 * M_PI / 180000000);
    float sin_lon2 = sin(lon2 * M_PI / 180000000);
    
    float x1 = RadiusEarth * cos_lat1 * cos_lon1;
    float y1 = RadiusEarth * cos_lat1 * sin_lon1;
    float x2 = RadiusEarth * cos_lat2 * cos_lon2;
    float y2 = RadiusEarth * cos_lat2 * sin_lon2;
    float z1 = RadiusEarth * sin_lat1;
    float z2 = RadiusEarth * sin_lat2;
    
    float delta_x = x2 - x1;
    float delta_y = y2 - y1;
    float delta_z = z2 - z1;
    
    float distance = sqrt((delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z));
    
    return (static_cast<uint32_t>(distance));
}
