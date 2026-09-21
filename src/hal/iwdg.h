/**
 * @file    iwdg.h
 * @brief   STM32H753 IWDG HAL implementation.
 */

#ifndef HAL_IWDG_H
#define HAL_IWDG_H

#include "cmsis/stm32h753xx.h"

static inline void enable_lsi( void )
{
    RCC->CSR |= RCC_CSR_LSION;

    while ((RCC->CSR & RCC_CSR_LSIRDY) == 0U)
    {

    }
}

static inline void iwdg_start( void )
{
    IWDG1->KR = 0xCCCCU;   /* start */
}

static inline void iwdg_unlock( void )
{
    IWDG1->KR = 0x5555U;   /* unlock PR and RLR */
}

static inline void iwdg_lock( void )
{
    IWDG1->KR = 0x0000U;   /* lock */
}

static inline void iwdg_set_prescaler( uint8_t pr )
{
    IWDG1->PR = pr & 0x07U;
}

static inline void iwdg_set_reload( uint16_t rlr )
{
    IWDG1->RLR = rlr & 0x0FFFU;
}

static inline void iwdg_refresh( void )
{
    IWDG1->KR = 0xAAAAU;
}

static inline void iwdg_wait_ready( void )
{
    while( ( IWDG1->SR & ( IWDG_SR_PVU | IWDG_SR_RVU ) ) != 0U )
    {
        /* wait for PR and RLR update to complete */
    }
}

#endif /* HAL_IWDG_H */
