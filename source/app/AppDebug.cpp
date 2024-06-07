#include "AppDebug.hpp"
#include "AppCore.hpp"

#include "peripherals.h"

void AppDebug::Init(void)
{
    // Do it here as it's buggy from the configurator
    CLOCK_SetClockDiv(kCLOCK_DivLPUART0, 12U);

    // Enable IRQ and redirect the interrupt handler
    EnableIRQ(LPUART0_SERIAL_RX_TX_IRQN);
    AppCore_SetInterruptHandler(LPUART0_IRQn, UartIrqHandler);

    channels_list[0] = this;
    uart_ptr         = LPUART0_PERIPHERAL;
}

bool AppDebug::Push(char c)
{
    return fifo.Push(c);
}

void AppDebug::Flush(void)
{
    if (!tx_ongoing && !fifo.IsEmpty())
    {
        uint32_t irqMask = DisableGlobalIRQ();
        // Enable send data register empty interrupt
        uart_ptr->CTRL |= (uint32_t)LPUART_CTRL_TIE_MASK;
        EnableGlobalIRQ(irqMask);

        tx_ongoing = true;
    }
}

void AppDebug::UartIrqHandler(void)
{
    AppDebug* this_ptr         = channels_list[0];
    uint32_t status            = LPUART_GetStatusFlags(this_ptr->uart_ptr);
    uint32_t enabledInterrupts = LPUART_GetEnabledInterrupts(this_ptr->uart_ptr);
    uint32_t irqMask;

    //! No Rx is forseen in here

    // Send data register empty and the interrupt is enabled
    if ((0U != ((uint32_t)kLPUART_TxDataRegEmptyFlag & status)) &&
        (0U != ((uint32_t)kLPUART_TxDataRegEmptyInterruptEnable & enabledInterrupts)))
    {
        // Get space in FIFO
        uint8_t max_count = (uint8_t)FSL_FEATURE_LPUART_FIFO_SIZEn(this_ptr->uart_ptr) - (uint8_t)((this_ptr->uart_ptr->WATER & LPUART_WATER_TXCOUNT_MASK) >> LPUART_WATER_TXCOUNT_SHIFT);

        // Transmit next bytes to FIFO
        char c;
        while (max_count-- && this_ptr->fifo.Pop(c))
        {
            this_ptr->uart_ptr->DATA = c;
        }

        // If we transferred the last byte
        if (this_ptr->fifo.IsEmpty())
        {
            irqMask = DisableGlobalIRQ();
            // Disable Tx register empty interrupt and enable transmission completion interrupt
            this_ptr->uart_ptr->CTRL = (this_ptr->uart_ptr->CTRL & ~LPUART_CTRL_TIE_MASK) | LPUART_CTRL_TCIE_MASK;
            EnableGlobalIRQ(irqMask);
        }
    }

    // End of transfer interrupt
    if ((0U != ((uint32_t)kLPUART_TransmissionCompleteFlag & status)) &&
        (0U != ((uint32_t)kLPUART_TransmissionCompleteInterruptEnable & enabledInterrupts)))
    {
        this_ptr->tx_ongoing = false;

        irqMask = DisableGlobalIRQ();
        // Disable transmission complete interrupt
        this_ptr->uart_ptr->CTRL &= ~(uint32_t)LPUART_CTRL_TCIE_MASK;
        EnableGlobalIRQ(irqMask);
    }
}
