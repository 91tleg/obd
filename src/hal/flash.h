/**
 * @file    flash.h
 * @brief   FLASH access control and wait-state configuration.
 */
 
#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdint.h>
#include "cmsis/stm32h753xx.h"
#include "hal/nvic.h"

static inline void flash_set_latency( uint32_t latency )
{
    FLASH->ACR = ( FLASH->ACR & ~FLASH_ACR_LATENCY_Msk )
               | ( latency << FLASH_ACR_LATENCY_Pos )
               | FLASH_ACR_WRHIGHFREQ_1;  /* required for VOS1 */

    /* Runs in system_init(), before SysTick/IWDG exist — see nvic.h. */
    HAL_SPIN_UNTIL_OR_RESET( ( FLASH->ACR & FLASH_ACR_LATENCY_Msk ) !=
                             ( latency << FLASH_ACR_LATENCY_Pos ) );
}

#endif /* HAL_FLASH_H */
