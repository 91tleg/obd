/**
 * @file    exti.h
 * @brief   EXTI and SYSCFG external interrupt configuration
 */

#ifndef HAL_EXTI_H
#define HAL_EXTI_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h753xx.h"

typedef enum
{
    EXTI_PORT_PA = 0U,
    EXTI_PORT_PB = 1U,
    EXTI_PORT_PC = 2U,
    EXTI_PORT_PD = 3U,
    EXTI_PORT_PE = 4U,
    EXTI_PORT_PF = 5U,
    EXTI_PORT_PG = 6U,
    EXTI_PORT_PH = 7U,
    EXTI_PORT_PI = 8U,
    EXTI_PORT_PJ = 9U,
    EXTI_PORT_PK = 10U,
} exti_port_t;

typedef enum
{
    EXTI_TRIGGER_RISING  = 0U,
    EXTI_TRIGGER_FALLING = 1U,
    EXTI_TRIGGER_BOTH    = 2U,
} exti_trigger_t;

/**
 * Select which GPIO port drives a given EXTI line.
 * pin must be 0..15.
 */
static inline void exti_set_port_source( exti_port_t port, uint8_t pin )
{
    if( pin < 16U )
    {
        uint32_t const reg_idx = ( uint32_t )pin / 4U;
        uint32_t const shift   = ( ( uint32_t )pin % 4U ) * 4U;

        SYSCFG->EXTICR[ reg_idx ] &= ~( 0xFUL << shift );
        SYSCFG->EXTICR[ reg_idx ] |=  ( ( uint32_t )port << shift );
    }
}

static inline void exti_set_trigger( uint8_t pin, exti_trigger_t trigger )
{
    if( pin < 16U )
    {
        uint32_t const mask = ( 1UL << ( uint32_t )pin );

        switch( trigger )
        {
            case EXTI_TRIGGER_RISING:
                EXTI->RTSR1 |=  mask;
                EXTI->FTSR1 &= ~mask;
                break;

            case EXTI_TRIGGER_FALLING:
                EXTI->FTSR1 |=  mask;
                EXTI->RTSR1 &= ~mask;
                break;

            case EXTI_TRIGGER_BOTH:
                EXTI->RTSR1 |= mask;
                EXTI->FTSR1 |= mask;
                break;

            default:
                break;
        }
    }
}

static inline void exti_unmask( uint8_t pin )
{
    if( pin < 16U )
    {
        EXTI->IMR1 |= ( 1UL << ( uint32_t )pin );
    }
}

static inline void exti_mask( uint8_t pin )
{
    if( pin < 16U )
    {
        EXTI->IMR1 &= ~( 1UL << ( uint32_t )pin );
    }
}

static inline bool exti_is_pending( uint8_t pin )
{
    bool result = false;

    if( pin < 16U )
    {
        result = ( ( EXTI->PR1 & ( 1UL << ( uint32_t )pin ) ) != 0U );
    }

    return result;
}

static inline void exti_clear_pending( uint8_t pin )
{
    if( pin < 16U )
    {
        /* PR1 is write-1-to-clear */
        EXTI->PR1 = ( 1UL << ( uint32_t )pin );
    }
}

#endif /* HAL_EXTI_H */
