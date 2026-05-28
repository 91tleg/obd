/**
 * @file    clock.h
 * @brief   STM32H753ZI clock init
 */

#ifndef BSP_NUCLEO_H753ZI_CLOCK_H
#define BSP_NUCLEO_H753ZI_CLOCK_H

#include <stdint.h>
#include "bsp/nucleo_h753zi/resources.h"

static inline uint32_t bsp_clock_sysclk_hz( void )
{
    return BSP_SYSCLK_HZ;
}

/**
 * Called from startup.s before system_init.
 * Exits Run* mode so PLL can be configured.
 */
void exit_run0_mode( void );

/**
 * Called from startup.s after exit_run0_mode.
 * Configures HSE, PLL1, prescalers, flash latency.
 */
void system_init( void );

#endif /* BSP_NUCLEO_H753ZI_CLOCK_H */
