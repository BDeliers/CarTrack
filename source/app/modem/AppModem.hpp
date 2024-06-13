#include "AppModemDriver.hpp"

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

    bool RetrieveGnssData(void);

private:
    bool gnss_has_fix{false};

};
