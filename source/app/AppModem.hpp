#pragma once

#include <array>

#include "fsl_lpuart_edma.h"

#include "log.h"

#include "utils/AtParser.hpp"

/// @brief Class to handle the LTE-M modem
class AppModem
{

public:
    /// @brief Initialize the modem communication
    void Init(void);

    /// @brief      Checks that the right modem is connected
    /// @return     True in case of successful detection
    bool DetectModem(void);

protected:
    /// @brief Available commands for the SIM7000 modem
    enum SIM7000_CMDSET {
        REQUEST_MODEM_IDENTIFICATION,
        SET_DATA_FLOW_CONTROL,
        SET_COMMAND_ECHO_MODE,
        SIM700_CMDSET_LEN,
    };

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
    bool RetrievePayload(char* buff, uint32_t buff_size);

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

    bool echo_enabled{true};

    static constexpr const char* SIM7000_OPCODES[SIM700_CMDSET_LEN] = {
        "CGMM",
        "IFC",
        "ATE",
    };
};
