/**
 * @file    flash.h
 * @brief   FLASH access control and wait-state configuration.
 */
 
#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdint.h>
#include "cmsis/stm32h753xx.h"

static inline void flash_set_latency( uint32_t latency )
{
    FLASH->ACR = ( FLASH->ACR & ~FLASH_ACR_LATENCY_Msk )
               | ( latency << FLASH_ACR_LATENCY_Pos )
               | FLASH_ACR_WRHIGHFREQ_1;  /* required for VOS1 */

    while( ( FLASH->ACR & FLASH_ACR_LATENCY_Msk ) !=
           ( latency << FLASH_ACR_LATENCY_Pos ) )
    {
        /* wait for latency to apply */
    }
}

#endif /* HAL_FLASH_H */
