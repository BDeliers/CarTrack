#include "AppModem.hpp"
#include "../AppCore.hpp"

#include "log.h"

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
        //SendCommandBlocking(GNSS_CONFIGURE_POSITION_ACCURACY, WRITE, 100, 1, "10");  // Set desired accuracy to 10m
        return true;
    }

    return false;
}

bool AppModem::RetrieveGnssData(void)
{
    if (SendCommandBlocking(GNSS_NAVIGATION_INFORMATION, EXEC, 100, 0))
    {
        char buffer[95];
        if (RetrievePayload(buffer, sizeof(buffer)))
        {
            if (buffer[12] == '1' && !gnss_has_fix)
            {
                gnss_has_fix = true;
                log_debug("GNSS fix found");
            }
            else if (buffer[12] == '0' && gnss_has_fix)
            {
                gnss_has_fix = false;
                log_debug("GNSS fix lost");
            }

            return true;
        }
    }

    return false;
}
