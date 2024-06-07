#pragma once

#include "MCXA153.h"

/// @brief Initialize the interrupt vector to RAM
void AppCore_InitInterruptVector(void);

/// @brief          Register a handler to an ISR
/// @param vector   ISR to register
/// @param handler  Handler function
void AppCore_SetInterruptHandler(IRQn_Type vector, void (*handler)(void));

uint64_t AppCore_GetUptimeMs(void);

void AppCore_BlockingDelayMs(uint32_t delay);
