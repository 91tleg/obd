/**
 * @file    main_hitl.c
 * @brief   Entry point for the CAN loopback HITL test.
 */

#include <stdint.h>

#include "bsp/nucleo_h753zi/board.h"
#include "bsp/nucleo_h753zi/wdt.h"
#include "lib/core/result.h"

uint32_t run_can_loopback_tests( void );
void hitl_finish( void );

extern volatile uint32_t hitl_fail;

int main( void )
{
    /* board_init() also starts the IWDG, which cannot be stopped again. */
    if( RES_IS_FAILED( board_init() ) )
    {
        hitl_fail++;
    }
    else
    {
        ( void )run_can_loopback_tests();
    }

    hitl_finish();

    /*
     * Keep feeding the watchdog. Returning would fall into the startup
     * `b .`, the IWDG would fire and the whole suite would silently re-run
     * every 2 s.
     */
    for( ;; )
    {
        bsp_wdt_refresh();
    }
}
