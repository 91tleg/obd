#ifndef HAL_SYSTICK_H
#define HAL_SYSTICK_H

#include <stdint.h>
#include "cmsis/stm32h753xx.h"

/**
 * Configure SysTick for 1ms tick at the given core clock frequency.
 * Does not start the tick counter — call systick_get_tick() after.
 */
static inline void systick_init( uint32_t core_clk_hz )
{
    SysTick->LOAD = ( core_clk_hz / 1000U ) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                  | SysTick_CTRL_TICKINT_Msk
                  | SysTick_CTRL_ENABLE_Msk;
}

#endif /* HAL_SYSTICK_H */
