#include "systick.h"
#include "lib/time/delay_tick.h"
#include "resources.h"
#include "hal/systick.h"

static volatile uint32_t s_tick = 0U;

/* satisfies delay_tick.h contract  */
uint32_t delay_get_tick( void )
{
    return s_tick;
}

void bsp_systick_init( void )
{
    systick_init( BSP_SYSCLK_HZ );
}

void systick_handler( void )
{
    ++s_tick;
}
