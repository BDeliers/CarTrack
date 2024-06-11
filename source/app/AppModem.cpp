#include "AppModem.hpp"
#include "AppCore.hpp"

#include "peripherals.h"
#include "pin_mux.h"

constexpr const char CR[]               = "\r";
constexpr const char CRLF[]             = "\r\n";
constexpr const char SIM7000_OK[]       = "OK";
constexpr const char SIM7000_ERROR[]    = "ERROR";
constexpr const char SIM7000_MODEL[]    = "SIMCOM_SIM7000G";

void AppModem::Init(void)
{
    // Initialize the peripherals pointers
    uart_ptr        = LPUART2_PERIPHERAL;
    uart_dma_handle = &LPUART2_LPUART_eDMA_Handle;
    dma_rx_handle   = &LPUART2_RX_Handle;
    dma_tx_handle   = &LPUART2_TX_Handle;

    // Register the end-of-transfer DMA callback. User data will be used to retrieve this object
    uart_dma_handle->callback = DmaTransferCallback;
    uart_dma_handle->userData = this;

    // Reset modem via its RST signal
    //! But don't, as it would reboot the LDO too, so power-cycle the MCU
    //GPIO_PinWrite(BOARD_INITPINS_MODEM_RST_GPIO, BOARD_INITPINS_MODEM_RST_PIN, 0);
    //AppCore_BlockingDelayMs(300);
    //GPIO_PinWrite(BOARD_INITPINS_MODEM_RST_GPIO, BOARD_INITPINS_MODEM_RST_PIN, 1);

    // Start to receive data
    StartReception();
}

bool AppModem::SendBuffer(void)
{
    // Don't mess the current transfer
    if (tx_started)
    {
        return false;
    }

    lpuart_transfer_t xfer = {
        .txData     = reinterpret_cast<uint8_t*>(buffer_tx),
        .dataSize   = strlen(buffer_tx),
    };
    
    // Send the TX buffer with DMA
    if (LPUART_SendEDMA(uart_ptr, uart_dma_handle, &xfer) == kStatus_Success)
    {
        tx_started = true;
        return true;
    }

    return false;
}

bool AppModem::StartReception(void)
{
    // No need to restart
    if (rx_started)
    {
        return false;
    }

    // Make the buffer zero-filled to be able to interpret result as strings
    memset(buffer_rx, 0, RX_BUFFER_SIZE);

    lpuart_transfer_t xfer = {
        .rxData     = reinterpret_cast<uint8_t*>(buffer_rx),
        .dataSize   = RX_BUFFER_SIZE,
    };

    // Start asynchronous reception thanks to the DMA
    if (LPUART_ReceiveEDMA(uart_ptr, uart_dma_handle, &xfer) == kStatus_Success)
    {
        rx_started = true;
        return true;
    }

    return false;
}

bool AppModem::ResetReception(void)
{
    // Abort the transfer then restart reception
    LPUART_TransferAbortReceiveEDMA(uart_ptr, uart_dma_handle);
    rx_started = false;

    return StartReception();
}

bool AppModem::BlockUntilOk(uint32_t timeout_ms)
{
    uint32_t cnt = 0;

    // Default is no-timeout
    uint64_t timeout_end = 0xFFFFFFFFFFFFFFFF;
    if (timeout_ms != 0xFFFFFFFF)
    {
        timeout_end = AppCore_GetUptimeMs() + timeout_ms;
    }

    do 
    {
        // When we have enough data in the buffer, check if it's "OK"
        if (LPUART_TransferGetReceiveCountEDMA(uart_ptr, uart_dma_handle, &cnt) == kStatus_Success 
            && cnt > strlen(SIM7000_OK))
        {
            // We don't use strcmp as the buffer still contains the CRLF
            if (memcmp((buffer_rx + cnt - strlen(SIM7000_OK) - strlen(CRLF)), SIM7000_OK, strlen(SIM7000_OK)) == 0)
            {
                return true;
            }   
        }
    }
    while (AppCore_GetUptimeMs() < timeout_end);

    return false;
}

bool AppModem::SendCommandBlocking(const SIM7000_CMDSET cmd, const CmdType type, const uint32_t timeout_ms, uint8_t params_count, ...)
{
    bool ret = false;

    va_list params;
    va_start(params, params_count);

    // Make the command buffer 
    if (AtParser_MakeCommand(type, buffer_tx, TX_BUFFER_SIZE, SIM7000_OPCODES[cmd], params_count, params) > 0)
    {
        // Reset reception to have a clean RX buffer, then send our command
        if (ResetReception() && SendBuffer())
        {
            // Wait for the status to be received
            if (BlockUntilOk(timeout_ms))
            {
                ret = true;
            }
        }
    }

    va_end(params);

    return ret;
}

bool AppModem::RetrievePayload(char* buff, uint32_t buff_size)
{
    uint32_t cnt = 0;

    if (LPUART_TransferGetReceiveCountEDMA(uart_ptr, uart_dma_handle, &cnt) == kStatus_Success
        && cnt > 0)
    {
        // We're after the OK\r\n
        char* payload_ptr_end = buffer_rx + cnt;
        // We're after the OK
        payload_ptr_end -= strlen(CRLF);
        // We're before the OK
        payload_ptr_end -= strlen(SIM7000_OK);
        // We're after the payload
        payload_ptr_end -= strlen(CRLF)*2;
        // We're at the last character of the payload
        payload_ptr_end--;

        char* payload_ptr_start = buffer_rx;

        // Get rid of the command echo
        if (echo_enabled)
        {
            // Locate the first CRLF which is after the command echo
            payload_ptr_start = strstr(buffer_rx, CRLF);

            if (payload_ptr_start == NULL)
            {
                return false;
            }

            // Payload starts after the command echo + CRLF (twice)
            payload_ptr_start += strlen(CRLF);
        }

        // If the buffer is big enough
        if (static_cast<uint32_t>(payload_ptr_end - payload_ptr_start) < buff_size)
        {
            // Reset it to 0 so it can be considered as a C-string
            memset(buff, 0, buff_size);
            // Copy the payload
            memcpy(buff, payload_ptr_start, payload_ptr_end - payload_ptr_start + 1);
            return true;
        }
    }

    return false;
}

void AppModem::DmaTransferCallback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    AppModem* this_ptr = static_cast<AppModem*>(handle->userData);

    // Tx finished
    if (this_ptr->tx_started && (status = kStatus_LPUART_TxIdle))
    {
        this_ptr->tx_started = false;
    }
    // Rx buffer full
    else if (this_ptr->rx_started && (status = kStatus_LPUART_RxIdle))
    {
        this_ptr->rx_started = false;
    }
}


bool AppModem::DetectModem(void)
{
    if (SendCommandBlocking(REQUEST_MODEM_IDENTIFICATION, EXEC, 100, 0) > 0)
    {
        char buff[sizeof(SIM7000_MODEL)];
        if (RetrievePayload(buff, sizeof(buff)))
        {
            if (strcmp(SIM7000_MODEL, buff) == 0)
            {
                return true;
            }
        }
    }

    return false;
}
