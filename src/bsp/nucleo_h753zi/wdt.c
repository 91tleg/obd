#include "bsp/nucleo_h753zi/wdt.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/iwdg.h"

/* LSI ~32kHz, prescaler /32 = 1kHz tick, reload 2000 = 2s timeout */
#define BSP_WDT_PRESCALER  ( 3U    )
#define BSP_WDT_RELOAD     ( 2000U )

void bsp_wdt_init( void )
{
    iwdg_unlock();
    iwdg_set_prescaler( BSP_WDT_PRESCALER );
    iwdg_set_reload( BSP_WDT_RELOAD );
    iwdg_wait_ready();
    iwdg_lock();
    iwdg_start();
    iwdg_refresh();
}

void bsp_wdt_refresh( void )
{
    iwdg_refresh();
}
