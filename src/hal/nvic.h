#ifndef HAL_NVIC_H
#define HAL_NVIC_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis/stm32h753xx.h"

/* 4 bits of priority — 0 highest, 15 lowest */
#define NVIC_PRIORITY_MAX   ( 0U )
#define NVIC_PRIORITY_MIN   ( 15U )
#define NVIC_PRIORITY_BITS  ( 4U )

static inline void nvic_enable_irq( IRQn_Type irq, uint8_t priority )
{
    if( priority <= NVIC_PRIORITY_MIN )
    {
        NVIC_SetPriority( irq, ( uint32_t )priority );
        NVIC_EnableIRQ( irq );
    }
}

static inline void nvic_disable_irq( IRQn_Type irq )
{
    NVIC_DisableIRQ( irq );
}

static inline void nvic_set_pending( IRQn_Type irq )
{
    NVIC_SetPendingIRQ( irq );
}

static inline void nvic_clear_pending( IRQn_Type irq )
{
    NVIC_ClearPendingIRQ( irq );
}

static inline bool nvic_is_pending( IRQn_Type irq )
{
    return ( NVIC_GetPendingIRQ( irq ) != 0UL );
}

static inline bool nvic_is_active( IRQn_Type irq )
{
    return ( NVIC_GetActive( irq ) != 0UL );
}

static inline void nvic_set_priority( IRQn_Type irq, uint8_t priority )
{
    if( priority <= NVIC_PRIORITY_MIN )
    {
        NVIC_SetPriority( irq, ( uint32_t )priority );
    }
}

static inline uint8_t nvic_get_priority( IRQn_Type irq )
{
    return ( uint8_t )NVIC_GetPriority( irq );
}

static inline void nvic_global_disable( void )
{
    __disable_irq();
}

static inline void nvic_global_enable( void )
{
    __enable_irq();
}

static inline uint32_t nvic_enter_critical( void )
{
    uint32_t const primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void nvic_exit_critical( uint32_t primask )
{
    __set_PRIMASK( primask );
}

#endif /* HAL_NVIC_H */
