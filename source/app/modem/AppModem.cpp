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
    AppModemDriver::Init();

    // Reset the modem
    /*AppCore_BlockingDelayMs(100);
    if (!SendCommandBlocking(RESET_DEFAULT_CONFIGURATION, EXEC, 100, 0))
    {
        return false;
    }*/

    // After a power-up, it can take up to some time for UART to be available
    AppCore_BlockingDelayMs(5000);

    // Try a baud rate synchronization
    if (SynchronizeBaudrate(255))
    {
    	AppCore_BlockingDelayMs(100);
        return DetectModem();
    }

    return false;
}

bool AppModem::DetectModem(void)
{
    if (SendCommandBlocking(REQUEST_MODEM_IDENTIFICATION, EXEC, 100, 0))
    {
        const char SIM7000_MODEL[] = "SIMCOM_SIM7000G";
        char buff[sizeof(SIM7000_MODEL)];
        if (RetrievePayload(buff, sizeof(buff)))
        {
            if (strcmp(SIM7000_MODEL, buff) == 0)
            {
                log_debug("SIM700G detected");
                return true;
            }
        }
    }

    return false;
}

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
