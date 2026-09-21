#include "bsp/nucleo_h753zi/wdt.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/iwdg.h"

/*
 * LSI ~32 kHz, PR=3 -> /32 = 1 kHz tick, reload 2000 = ~2 s timeout.
 * IWDG prescaler: 0=/4 1=/8 2=/16 3=/32 4=/64 5=/128 6=/256.
 */
#define BSP_WDT_PRESCALER  ( 3U )
#define BSP_WDT_RELOAD     ( 2000U )

void bsp_wdt_init( void )
{
    enable_lsi();

    iwdg_start();
    iwdg_unlock();

    iwdg_set_prescaler( BSP_WDT_PRESCALER );
    iwdg_set_reload( BSP_WDT_RELOAD );

    iwdg_wait_ready();

    iwdg_refresh();
}

void bsp_wdt_refresh( void )
{
    iwdg_refresh();
}
