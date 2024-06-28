#pragma once

#include <array>

#include "fsl_lpuart_edma.h"

#include "log.h"

#include "utils/AtParser.hpp"

/// @brief Low-level driver for the SIM7000G modem
class AppModemDriver
{

protected:
    /// @brief  Initialize the modem communication
    /// @return False if init failed
    bool Init(void);
    
    /// @brief Available commands for the SIM7000 modem
    enum SIM7000_CMDSET {
        REQUEST_MODEM_IDENTIFICATION,
        SET_DATA_FLOW_CONTROL,
        SET_COMMAND_ECHO_MODE,
        RESET_DEFAULT_CONFIGURATION,
        CONTROL_GPIO,
        PREFERRED_MODE_SELECTION,
        PREFERRED_SELECTION_LTE,
        APP_NETWORK_ACTIVE,
        PDP_CONFIGURE,
        GNSS_POWER_CONTROL,
        GNSS_NAVIGATION_INFORMATION,
        GNSS_WORK_MODE_SET,
        GNSS_COLD_START,
        GNSS_CONFIGURE_POSITION_ACCURACY,
        GPP3_REPORT_MOBILE_EQUIPEMENT_ERROR,
        GPP3_MOBILE_ENTER_PIN,
        GPP3_NETWORK_REGISTRATION,
        GPP3_SIGNAL_QUALITY_REPORT,
        GPP3_SET_PHONE_FUNCTIONNALITY,
        GPP3_OPERATOR_SELECTION,
        GPRS_ATTACH_DETACH_SERVICE,
        GPRS_DEFINE_PDP_CONTEXT,
        GPRS_NETWORK_REGISTRATION_STATUS,
        IP_BEARER_SETTINGS,
        TCPIP_START_TASK_SET_APN,
        TCPIP_BRING_UP_CONNECTION,
        TCPIP_SHUTDOWN_CONNECTION,
        PING_SEND_IPV4,
        SIM700_CMDSET_LEN,
    };

    /// @brief          Tries to make the modem detect the MCU's baudrate (cf UART application note)
    /// @param n_tries  Number of retries
    /// @return         True if success
    bool SynchronizeBaudrate(uint8_t n_tries);

    /// @brief              Send a command and block until success or timeout
    /// @param cmd          Command to be sent to the modem
    /// @param type         Type of command
    /// @param timeout_ms   Timeout to block for, in ms. 0xFFFFFFFF means no timeout
    /// @param params_count Number of va_args
    /// @param              va_args
    /// @return             True if the OK status was returned
    bool SendCommandBlocking(const SIM7000_CMDSET cmd, const CmdType type, const uint32_t timeout_ms, const uint8_t params_count, ...);
    
    /// @brief              Retrieve the returned payload
    /// @param buff         Buffer to fill with the payload
    /// @param buff_size    Number of bytes copied
    /// @return             True in case of success
    bool RetrievePayload(char* buff, const uint32_t buff_size);

    // -- GENERIC ACTIONS

    /// @brief      Checks that the right modem is connected
    /// @return     True in case of successful detection
    bool DetectModem(void);

    /// @brief          Enabled detailed verbose debug info
    /// @param enable   True to enable, false to disable
    /// @return         True in case of success
    bool EnableExtendedErorr(bool enable);

    bool echo_enabled{true};

private:
    static void DmaTransferCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData);

    /// @brief  Send the TX buffer contents to the mode
    ///         Non-blocking
    /// @return True if the transfer started properly
    bool SendBuffer(void);

    /// @brief  Start the asynchronous reception
    ///         Non-blocking
    /// @return True if the reception started properly
    bool StartReception(void);

    /// @brief  Resets the RX buffer and restarts reception
    /// @return True if the reception restarted properly
    bool ResetReception(void);

    /// @brief              Block until the OK status is not received or timeout
    /// @param timeout_ms   Timeout in ms. 0xFFFFFFFF means no timeout
    /// @return             True if the OK was received properly
    bool BlockUntilOk(uint32_t timeout_ms);

    LPUART_Type* uart_ptr{NULL};

    lpuart_edma_handle_t*   uart_dma_handle;
    edma_handle_t*          dma_tx_handle;
    edma_handle_t*          dma_rx_handle;

    static constexpr const uint16_t TX_BUFFER_SIZE = 512;
    static constexpr const uint16_t RX_BUFFER_SIZE = 512;

    char buffer_tx[TX_BUFFER_SIZE];
    char buffer_rx[RX_BUFFER_SIZE];

    volatile bool tx_started{false};
    volatile bool rx_started{false};

    static constexpr const char* SIM7000_OPCODES[SIM700_CMDSET_LEN] = {
        "CGMM",         // REQUEST_MODEM_IDENTIFICATION
        "IFC",          // SET_DATA_FLOW_CONTROL
        "ATE",          // SET_COMMAND_ECHO_MODE
        "ATZ",          // RESET_DEFAULT_CONFIGURATION
        "SGPIO",        // CONTROL_GPIO
        "CNMP",         // PREFERRED_MODE_SELECTION
        "CMNB",         // PREFERRED_SELECTION_LTE
        "CNACT",        // APP_NETWORK_ACTIVE
        "CNCFG",        // PDP_CONFIGURE
        "CGNSPWR",      // GNSS_POWER_CONTROL
        "CGNSINF",      // GNSS_NAVIGATION_INFORMATION
        "CGNSMOD",      // GNSS_WORK_MODE_SET
        "CGNSCOLD",     // GNSS_COLD_START
        "CGNSHOR",      // GNSS_CONFIGURE_POSITION_ACCURACY
        "CMEE",         // GPP3_REPORT_MOBILE_EQUIPEMENT_ERROR
        "CPIN",         // GPP3_MOBILE_ENTER_PIN
        "CREG",         // GPP3_NETWORK_REGISTRATION
        "CSQ",          // GPP3_SIGNAL_QUALITY_REPORT
        "CFUN",         // GPP3_SET_PHONE_FUNCTIONNALITY
        "COPS",         // GPP3_OPERATOR_SELECTION
        "CGATT",        // GPRS_ATTACH_DETACH_SERVICE
        "CGDCONT",      // GPRS_DEFINE_PDP_CONTEXT
        "CGREG",        // GPRS_NETWORK_REGISTRATION_STATUS
        "SAPBR",        // IP_BEARER_SETTINGS
        "CSTT",         // TCPIP_START_TASK_SET_APN
        "CIICR",        // TCPIP_BRING_UP_CONNECTION
        "CIPSHUT",      // TCPIP_SHUTDOWN_CONNECTION
        "SNPING4",      // PING_SEND_IPV4
    };

};
