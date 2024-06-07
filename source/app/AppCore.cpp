// --- INCLUDES ---
#include "AppCore.hpp"

#include <cstring>
#include <ctime>
#include "fsl_common_arm.h"


// --- LOCAL VARIABLES ---
extern void (* const g_pfnVectors[])(void);

__attribute__ ((used, section(".ram_vector")))
static void (*isr_table[NUMBER_OF_INT_VECTORS])(void);

static volatile uint64_t tick_count = 0;

// --- STATIC FUNCTIONS PROTOTYPES ---
static void SysTick_Handler(void);

// --- STATIC FUNCTIONS ---
static void SysTick_Handler(void)
{
    tick_count++;
}

extern "C"
{

/// @brief C-Library timestamp server
time_t time (time_t *__timer)
{
    return (tick_count / 1000);
}

}

void AppCore_InitInterruptVector(void)
{
    memcpy(isr_table, g_pfnVectors, sizeof(isr_table));

    isr_table[SysTick_IRQn+16] = SysTick_Handler;

    __disable_irq();
    SCB->VTOR = (uint32_t)&isr_table;
    __enable_irq();

}

void AppCore_SetInterruptHandler(IRQn_Type vector, void (*handler)(void))
{
    if (vector >= 0 && handler != NULL)
    {
        uint32_t irqMask = DisableGlobalIRQ();
        isr_table[vector + 16] = handler;
        EnableGlobalIRQ(irqMask);
    }
}

uint64_t AppCore_GetUptimeMs(void)
{
    return tick_count;
}

void AppCore_BlockingDelayMs(uint32_t delay)
{
    uint64_t end = tick_count + delay;

    while (tick_count != end);
}
