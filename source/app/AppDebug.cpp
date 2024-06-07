#include "AppDebug.hpp"

#include "peripherals.h"

bool AppDebug::CircularBuffer::Push(char& c)
{
    if (this->IsFull())
    {
        return false;
    }

    buffer[tail++] = c;

    tail %= size;
    usage++;

    return true;
}

bool AppDebug::CircularBuffer::Pop(char& c)
{
    if (this->IsEmpty())
    {
        return false;
    }

    c = buffer[head++];

    head %= size;
    usage--;

    return true;
}

bool AppDebug::CircularBuffer::IsFull(void)
{
    return (usage == size);
}

bool AppDebug::CircularBuffer::IsEmpty(void)
{
    return (usage == 0);
}

void AppDebug::Init(void)
{
    // Do it here as it's buggy from the configurator
    CLOCK_SetClockDiv(kCLOCK_DivLPUART0, 12U);

    uart_ptr = LPUART0_PERIPHERAL;
    EnableIRQ(LPUART0_SERIAL_RX_TX_IRQN);
}

bool AppDebug::Push(char c)
{
    return fifo.Push(c);
}

void AppDebug::Flush(void)
{
    if (!tx_ongoing && !fifo.IsEmpty())
    {
        // Enable send data register empty interrupt
        uint32_t irqMask = DisableGlobalIRQ();
        // Enable transmitter interrupt
        uart_ptr->CTRL |= (uint32_t)LPUART_CTRL_TIE_MASK;
        EnableGlobalIRQ(irqMask);
        tx_ongoing = true;
    }
}

void AppDebug::UartIrqHandler(void)
{
    uint32_t status            = LPUART_GetStatusFlags(uart_ptr);
    uint32_t enabledInterrupts = LPUART_GetEnabledInterrupts(uart_ptr);

    //! No Rx is forseen in here

    // Send data register empty and the interrupt is enabled
    if ((0U != ((uint32_t)kLPUART_TxDataRegEmptyFlag & status)) &&
        (0U != ((uint32_t)kLPUART_TxDataRegEmptyInterruptEnable & enabledInterrupts)))
    {
        uint8_t max_count = (uint8_t)FSL_FEATURE_LPUART_FIFO_SIZEn(uart_ptr) - (uint8_t)((uart_ptr->WATER & LPUART_WATER_TXCOUNT_MASK) >> LPUART_WATER_TXCOUNT_SHIFT);

        // Transmit next bytes to FIFO
        char c;
        while (max_count-- && fifo.Pop(c))
        {
            uart_ptr->DATA = c;
        }

        // Transmission finished, just wait for the last byte to be transferred
        if (fifo.IsEmpty())
        {
            uint32_t irqMask = DisableGlobalIRQ();
            // Disable Tx register empty interrupt and enable transmission completion interrupt
            uart_ptr->CTRL = (uart_ptr->CTRL & ~LPUART_CTRL_TIE_MASK) | LPUART_CTRL_TCIE_MASK;
            EnableGlobalIRQ(irqMask);
        }
    }

    // End of transfer interrupt
    if ((0U != ((uint32_t)kLPUART_TransmissionCompleteFlag & status)) &&
        (0U != ((uint32_t)kLPUART_TransmissionCompleteInterruptEnable & enabledInterrupts)))
    {
        tx_ongoing = false;

        uint32_t irqMask = DisableGlobalIRQ();
        // Disable transmission complete interrupt
        uart_ptr->CTRL &= ~(uint32_t)LPUART_CTRL_TCIE_MASK;
        EnableGlobalIRQ(irqMask);
    }
}

extern "C"
{

int __io_putchar(int ch)
{
    return (AppDebug::GetInstance().Push(ch) ? ch : -1);
}

void LPUART0_IRQHandler(void)
{
    AppDebug::GetInstance().UartIrqHandler();
}

}
