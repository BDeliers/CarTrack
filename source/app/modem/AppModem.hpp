#include "AppModemDriver.hpp"

#include "utils/GpsInfo.hpp"

class AppModem : private AppModemDriver
{

public:
    /// @brief Initialize the modem
    bool Init(void);

    // -- GENERIC ACTIONS

    // -- GNSS RELATED ACTIONS

    /// @brief          Power up/down the GNSS capabilities of the modem
    /// @param enable   Enable/disable
    /// @return         True in case of success
    bool EnableGnss(bool enable);

    /// @brief      Retrieve the GNSS data
    /// @param info GpsInfo object to fill-in
    /// @return     True if data has been acquired and parsed
    bool RetrieveGnssData(GpsInfo& info);

    // -- MOBILE RELATED ACTIONS

    /// @brief      Set the modem phone mode
    /// @param mode Full (true) or offline (false) mode
    /// @return     True in case of success
    bool SetPhoneMode(bool mode);

    /// @brief  Checks the SIM card status
    /// @return True if the SIM card is ready to operate, false in any other case
    bool CheckSimReady(void);

    /// @brief  Checks the network registration status
    /// @return True if a network is registered (roaming or not) and ready to use, false otherwise
    bool CheckNetworkRegistration(void);

    /// @brief  Configure LTE only operation
    /// @return True in case of success 
    bool ConfigureLte(void);

    bool ListOperators(void);

    /// @brief  Retrieve the signal quality
    /// @return RSSI from 0 (lowest) to 31 (best). 99 means no network.
    uint8_t CheckSignalQuality(void);

    bool EnableGprs(bool enable, char* apn, char* user, char* password);

    // -- IP RELATED ACTIONS

    bool EnableAppNetwork(bool enable);

    bool GetIp(char* ip);

    bool SendPingRequest(char* url);

private:
    bool gnss_has_fix{false};
    bool network_attached{false};
    bool network_configured{false};
    bool app_network_enabled{false};

    char retained_apn[128]{0};

};
