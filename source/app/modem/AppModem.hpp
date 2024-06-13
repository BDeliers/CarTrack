#include "AppModemDriver.hpp"

#include "utils/GpsInfo.hpp"

class AppModem : private AppModemDriver
{

public:
    /// @brief Initialize the modem
    bool Init(void);

    /// @brief      Checks that the right modem is connected
    /// @return     True in case of successful detection
    bool DetectModem(void);

    /// @brief          Power up/down the GNSS capabilities of the modem
    /// @param enable   Enable/disable
    /// @return         True in case of success
    bool EnableGnss(bool enable);

    /// @brief      Retrieve the GNSS data
    /// @param info GpsInfo object to fill-in
    /// @return     True if data has been acquired and parsed
    bool RetrieveGnssData(GpsInfo& info);

private:
    bool gnss_has_fix{false};

};
