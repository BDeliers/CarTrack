#include "AppModem.hpp"
#include "../AppCore.hpp"

#include "log.h"

#include <cstdlib>

extern "C"
{
    char *strptime(const char *buf, const char *fmt, struct tm *tm);
}

bool AppModem::Init(void)
{
    if (AppModemDriver::Init())
    {
        return true;
    }

    return false;
}

// -- GENERIC ACTIONS

// -- GNSS RELATED ACTIONS

bool AppModem::EnableGnss(bool enable)
{
    if (SendCommandBlocking(GNSS_WORK_MODE_SET, WRITE, 100, 4, "1", "1", "1", "1")      // Enable GLONASS, BEIDOU, GALILEO
        && SendCommandBlocking(CONTROL_GPIO, WRITE, 100, 4, "0", "4", "1", "1")         // Set GPIO 4 high to power antennas on my modified TE kit
        && SendCommandBlocking(GNSS_POWER_CONTROL, WRITE, 100, 1, enable ? "1" : "0"))  // Enable GNSS
    {
        return true;
    }

    return false;
}

bool AppModem::RetrieveGnssData(GpsInfo& info)
{
    if (SendCommandBlocking(GNSS_NAVIGATION_INFORMATION, EXEC, 100, 0))
    {
        // Useful payload is 95 bytes but payload starts with "+CGNSINF: "
        char buffer[105];
        if (RetrievePayload(buffer, sizeof(buffer)))
        {
            // First, check if we have a GPS fix
            info.SetFix(buffer[12] == '1' ? true : false);

            if (info.GetFix() && !gnss_has_fix)
            {
                gnss_has_fix = true;
                log_debug("GNSS fix found");
            }
            else if (!info.GetFix() && gnss_has_fix)
            {
                gnss_has_fix = false;
                log_debug("GNSS fix lost");
            }

            // Parse data only if we have a fix
            if (gnss_has_fix)
            {
                // Array of payload pointers. Cf AT commands manual table 15-1
                char* payload_ptr[21] = {0};

                // Split the buffer on the commas
                payload_ptr[0] = &buffer[10];
                uint8_t i = 1;
                for (char* ptr = &buffer[10]; ptr < (buffer + sizeof(buffer)); ptr++)
                {
                    if (*ptr == ',')
                    {
                        payload_ptr[i++] = ptr+1;
                    }
                }

                // Parse time and store it as timestamp only
                struct tm datetime;
                strptime(payload_ptr[2], "%Y%m%d%H%M%S", &datetime);
                info.SetTimestamp(mktime(&datetime));

                // Store each float parameter as an uint with a multiplication ratio. In case of empty, we'll get a zero
                info.SetLatitude(static_cast<int32_t>(strtod(payload_ptr[3], NULL) * 1e6));
                info.SetLongitude(static_cast<int32_t>(strtod(payload_ptr[4], NULL) * 1e6));
                info.SetAltitude(static_cast<uint32_t>(strtod(payload_ptr[5], NULL) * 1e3));
                info.SetSpeed(static_cast<uint16_t>(strtod(payload_ptr[6], NULL) * 1e2));
                info.SetCourse(static_cast<uint16_t>(strtod(payload_ptr[7], NULL) * 1e2));
                info.SetHdop(static_cast<uint16_t>(strtod(payload_ptr[10], NULL) * 10));
                info.SetPdop(static_cast<uint16_t>(strtod(payload_ptr[11], NULL) * 10));
                info.SetVdop(static_cast<uint16_t>(strtod(payload_ptr[12], NULL) * 10));
                info.SetSatellitesInView(static_cast<uint8_t>(strtod(payload_ptr[14], NULL)));

                // Sum the GPS & GLONASS satellites
                uint16_t sat_sum = static_cast<uint8_t>(strtod(payload_ptr[15], NULL));
                sat_sum += static_cast<uint8_t>(strtod(payload_ptr[16], NULL));
                info.SetSatellitesInUse(sat_sum);

                info.SetCn0(static_cast<uint8_t>(strtoul(payload_ptr[18], NULL, 10)));
            }

            return true;
        }
    }

    return false;
}

// -- MOBILE RELATED ACTIONS

bool AppModem::SetPhoneMode(bool mode)
{
    
    if (SendCommandBlocking(GPP3_SET_PHONE_FUNCTIONNALITY, WRITE, 10e3, 2, mode ? "1" : "0", "0"))  // Full functionnality OR offline, no reset
    {
        return true;
    }

    return false;
}

bool AppModem::CheckSimReady(void)
{
    if (SendCommandBlocking(GPP3_MOBILE_ENTER_PIN, READ, 100, 0))
    {
        const char SIM7000_READY[] = "+CPIN: READY";
        char buff[sizeof(SIM7000_READY)];
        if (RetrievePayload(buff, sizeof(buff)))
        {
            if (strcmp(SIM7000_READY, buff) == 0)
            {
                log_debug("SIM is ready");
                return true;
            }
        }
    }

    return false;
}

bool AppModem::CheckNetworkRegistration(void)
{
    if (SendCommandBlocking(GPRS_NETWORK_REGISTRATION_STATUS, READ, 100, 0))
    {
        char buff[16];
        if (RetrievePayload(buff, sizeof(buff)))
        {
            // Home network OR roaming
            if (buff[10] == '1' || buff[10] == '5')
            {
                if (!network_attached)
                {
                    network_attached = true;
                    log_debug("Attached to network");
                    return true;
                }
            }
            else if (network_attached)
            {
                network_attached = false;
                log_debug("Detached from network");
            }
        }
    }

    return false;
}

bool AppModem::ConfigureLte(void)
{
    if (SendCommandBlocking(PREFERRED_MODE_SELECTION, WRITE, 100, 1, "38")       // 2 = auto, 13 = GPRS only, 38 = LTE mode only
        && SendCommandBlocking(PREFERRED_SELECTION_LTE, WRITE, 100, 1, "1"))    // LTE-M only
    {
        // Apply change with RF restart
        if (SetPhoneMode(false) && SetPhoneMode(true))
        {
            AppCore_BlockingDelayMs(1e3);
            return true;
        }
    }

    return false;
}

bool AppModem::ListOperators(void)
{
    if (SendCommandBlocking(GPP3_OPERATOR_SELECTION, TEST, 45e3, 0))
    {
        return true;
    }

    return false;
}

uint8_t AppModem::CheckSignalQuality(void)
{
    if (SendCommandBlocking(GPP3_SIGNAL_QUALITY_REPORT, EXEC, 100, 0))
    {
        char buff[12];
        if (RetrievePayload(buff, sizeof(buff)))
        {
            uint8_t sq = static_cast<uint8_t>(strtoul(&buff[6], NULL, 10));
            return (sq == 99 ? 0xFF : sq);
        }
    }

    return 0xFF;
}

bool AppModem::EnableGprs(bool enable, char* apn, char* user, char* password)
{
    if (enable)
    {
        if(SendCommandBlocking(GPRS_DEFINE_PDP_CONTEXT, WRITE, 1e3, 3, "1", "\"IP\"", apn)                  // Set PDP context for GPRS
            && SendCommandBlocking(GPRS_ATTACH_DETACH_SERVICE, WRITE, 75e3, 1, "1"))                        // Attach GPRS service
        {
            if (SendCommandBlocking(TCPIP_SHUTDOWN_CONNECTION, EXEC, 1e3, 0)                                // Close all TCPIP connections
                && SendCommandBlocking(IP_BEARER_SETTINGS, WRITE, 1e3, 4, "3", "1", "\"APN\"", apn)         // Set APN for bearer 1
                && SendCommandBlocking(TCPIP_START_TASK_SET_APN, WRITE, 1e3, 3, apn, user, password)        // Set APN, user and pass for IP operations
                && SendCommandBlocking(IP_BEARER_SETTINGS, WRITE, 1e3, 4, "3", "1", "\"USER\"", user)       // Set username for bearer 1
                && SendCommandBlocking(IP_BEARER_SETTINGS, WRITE, 1e3, 4, "3", "1", "\"PWD\"", password)    // Set password for bearer 1
                && SendCommandBlocking(IP_BEARER_SETTINGS, WRITE, 85e3, 2, "1", "1")                        // Open bearer
                && SendCommandBlocking(TCPIP_BRING_UP_CONNECTION, EXEC, 85e3, 0))                           // Start TCPIP connection
            {
                log_debug("Network configured");
                network_configured = true;
                strncpy(retained_apn, apn, sizeof(retained_apn) - 1);
                return true;
            }
        }
    }
    else
    {
        if (SendCommandBlocking(TCPIP_SHUTDOWN_CONNECTION, EXEC, 1e3, 0)                // Close TCPIP connection
            && SendCommandBlocking(IP_BEARER_SETTINGS, WRITE, 1e3, 2, "0", "1")         // Close bearer 1
            && SendCommandBlocking(GPRS_ATTACH_DETACH_SERVICE, WRITE, 65e3, 1, "0"))    // Detach GPRS
        {
            log_debug("Network unconfigured");
            memset(apn, 0, sizeof(apn));
            network_configured = false;
            return true;
        }
    }

    return false;
}

bool AppModem::EnableAppNetwork(bool enable)
{
    if (enable)
    {
        if (SendCommandBlocking(APP_NETWORK_ACTIVE, WRITE, 1e3, 2, "1", this->retained_apn))
        {
            app_network_enabled = true;
            return true;
        }
    }
    else
    {
        if (SendCommandBlocking(APP_NETWORK_ACTIVE, WRITE, 1e3, 1, "0"))
        {
            app_network_enabled = false;
            return true;
        }
    }

    return false;
}

bool AppModem::GetIp(char* ip)
{
    if (app_network_enabled
        && SendCommandBlocking(APP_NETWORK_ACTIVE, READ, 1e3, 0))
    {
        char buff[30];
        if (RetrievePayload(buff, sizeof(buff)))
        {
            char* ip_start = strstr(buff, "\"") +1;
            char* ip_end = strstr(ip_start, "\"");
            *ip_end = 0;

            strcpy(ip, ip_start);

            return true;
        }
    }

    return false;
}


bool AppModem::SendPingRequest(char* url)
{
    if (app_network_enabled
        && SendCommandBlocking(PING_SEND_IPV4, WRITE, 1e3, 4, url, "1", "1", "1000"))
    {
        return true;
    }

    return false;
}
