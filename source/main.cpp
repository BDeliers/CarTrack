/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    CarTrack.cpp
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MCXA153.h"
/* TODO: insert other include files here. */
#include "app/AppMain.hpp"
#include "app/AppCore.hpp"

/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */
int main(void) {

    AppCore_InitInterruptVector();

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();

    AppMain::GetInstance().MainLoop();

    return 0 ;
}
