#include "delay.h"
#include <stddef.h>
#include "stm32h753xx.h"

static volatile uint32_t s_tick = 0U;

void delay_init( void )
{
    extern uint32_t SystemCoreClock;

    SysTick->LOAD = ( SystemCoreClock / 1000U ) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk  /* processor clock       */
                  | SysTick_CTRL_TICKINT_Msk    /* enable interrupt      */
                  | SysTick_CTRL_ENABLE_Msk;    /* start                 */

    s_tick = 0U;
}

void delay_tick( void )
{
    ++s_tick;
}

uint32_t delay_get_tick( void )
{
    return s_tick;
}

void delay_ms( uint32_t ms )
{
    uint32_t const start = s_tick;

    while( ( s_tick - start ) < ms )
    {
        /* wait — SysTick ISR increments s_tick */
    }
}

void delay_us( uint32_t us )
{
    extern uint32_t SystemCoreClock;

    uint32_t const cycles_per_us = SystemCoreClock / 1000000U;
    uint32_t const total_cycles  = us * cycles_per_us;

    volatile uint32_t i = 0U;
    while( i < total_cycles )
    {
        ++i;
    }
}

void delay_timer_start( delay_timer_t * p_timer, uint32_t duration_ms )
{
    if( p_timer != NULL )
    {
        p_timer->start_tick  = s_tick;
        p_timer->duration_ms = duration_ms;
        p_timer->running     = true;
    }
}

bool delay_timer_expired( delay_timer_t * p_timer )
{
    bool result = false;

    if( p_timer != NULL )
    {
        if( p_timer->running )
        {
            result = ( ( s_tick - p_timer->start_tick ) >= p_timer->duration_ms );

            if( result )
            {
                p_timer->running = false;
            }
        }
    }

    return result;
}

void delay_timer_reset( delay_timer_t * p_timer )
{
    if( p_timer != NULL )
    {
        p_timer->start_tick = s_tick;
        p_timer->running    = true;
    }
}

void delay_timer_stop( delay_timer_t * p_timer )
{
    if( p_timer != NULL )
    {
        p_timer->running = false;
    }
}

uint32_t delay_timer_elapsed( delay_timer_t * p_timer )
{
    uint32_t result = 0U;

    if( ( p_timer != NULL ) && ( p_timer->running ) )
    {
        result = s_tick - p_timer->start_tick;
    }

    return result;
}

bool delay_timer_running( const delay_timer_t * p_timer )
{
    bool result = false;

    if( p_timer != NULL )
    {
        result = p_timer->running;
    }

    return result;
}
