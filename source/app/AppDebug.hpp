#pragma once

#include <array>

#include "fsl_lpuart.h"

#include "log.h"

#include "utils/CircularBuffer.hpp"

/// @brief Class to handle the global debug channel
class AppDebug
{

public:
    /// @brief Initialize the debug channel
    void Init(void);

    /// @brief      Push a character to the debug peripheral
    /// @param c    Character to push
    /// @return     True if success
    bool Push(char c);

    /// @brief Flush the debug peripheral
    void Flush(void);


private:

    CircularBuffer fifo;

    LPUART_Type* uart_ptr{NULL};
    volatile bool tx_ongoing{false};

    inline static AppDebug* channels_list[1];

    static void UartIrqHandler(void);

};
