/**
 * @file    main_hitl.c
 * @brief   Entry point for the CAN loopback HITL test.
 */

#include "bsp/nucleo_h753zi/board.h"

void run_can_loopback_tests( void );

int main( void )
{
    board_init();

    run_can_loopback_tests();

    return 0;
}
