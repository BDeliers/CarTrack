#include "AppModemDriver.hpp"

#include "utils/GpsInfo.hpp"

/// @brief  Macro to add escaped quote to a string, transforming "string" to "\"string\""
#define QUOTE(str) "\"" str "\""

class AppModem : private AppModemDriver
{

public:
    enum class ReqMethod
    {
        GET = 1,
        POST,
        PUT,
        DELETE,
    };

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

    /// @brief          Enable the GPRS communication
    /// @param enable   Enable or disable
    /// @param apn      APN, between quotes (like "\"my.apn.com\"")
    /// @param user     Username, quoted same as APN
    /// @param password Password, quoted same as APN
    /// @return         True in case of success
    bool EnableGprs(bool enable, char* apn, char* user, char* password);

    // -- IP RELATED ACTIONS

    /// @brief          Activate or de-activate the IP connectivity
    /// @param enable   Enable or disable
    /// @return         True in case of success
    bool EnableAppNetwork(bool enable);

    /// @brief      Get the module's IP address
    /// @param ip   Buffer to fill in
    /// @return     True in case of success
    bool GetIp(char* ip);

    /// @brief      Send a PING to the given URL
    /// @note       This doesn't work, for some unknown reasons
    /// @param url  URL to ping
    /// @return     True in case of success
    bool SendPingRequest(char* url);

    // -- COAP RELATED ACTIONS

    /// @brief          Send a CoAp request
    /// @param url      URL to send the request to, between quotes (like "\"my.url.com\"")
    /// @param method   Method to use (GET, POST, PUT, DELETE)
    /// @param uri      URI below the URL, quoted same as URL
    /// @param query    Query parameters below the URI, quoted same as URL
    /// @param payload  Payload to be sent, quoted same as URL
    /// @return         True in case of success
    bool SendCoapRequest(const char* url, ReqMethod method, char* uri, char* query, char* payload);

private:
    bool gnss_has_fix{false};
    bool network_attached{false};
    bool network_configured{false};
    bool app_network_enabled{false};

    char retained_apn[128]{0};

};
