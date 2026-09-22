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

/*
 * Generous iteration bound for HAL_SPIN_UNTIL_OR_RESET(). Not a calibrated
 * timeout — the condition normally clears in microseconds to low-single-digit
 * milliseconds — just large enough to never trip on working hardware while
 * still being bounded.
 */
#define HAL_BOOT_SPIN_LIMIT  ( 2000000UL )

/**
 * Busy-wait on a hardware condition that must clear during early boot —
 * clock/power bring-up in system_init(), or LSI/IWDG bring-up in
 * bsp_wdt_init() — before SysTick or the IWDG are running. With no timebase
 * and no watchdog yet, an unconditional "while(cond){}" here is a permanent,
 * unrecoverable hang if the hardware never responds (dead crystal, etc.).
 *
 * Spins up to HAL_BOOT_SPIN_LIMIT iterations; if @p cond is still true when
 * the count is exhausted, forces a system reset instead of hanging forever,
 * so a transient issue gets a fresh boot attempt rather than a dead board.
 */
#define HAL_SPIN_UNTIL_OR_RESET( cond )                                \
    do {                                                              \
        uint32_t _hal_spin = HAL_BOOT_SPIN_LIMIT;                     \
        while( ( cond ) && ( _hal_spin > 0U ) )                       \
        {                                                             \
            --_hal_spin;                                             \
        }                                                             \
        if( _hal_spin == 0U )                                        \
        {                                                             \
            NVIC_SystemReset();                                      \
        }                                                             \
    } while( 0 )

#endif /* HAL_NVIC_H */
