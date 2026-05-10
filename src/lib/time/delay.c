#include "delay.h"
#include "delay_tick.h"

static uint32_t s_cycles_per_us = 0U;

void delay_init( uint32_t core_clk_hz )
{
    s_cycles_per_us = core_clk_hz / 1000000U;
}

void delay_ms( uint32_t ms )
{
    uint32_t const start = delay_get_tick();
    while( ( delay_get_tick() - start ) < ms )
    {
        /* wait — SysTick ISR increments tick */
    }
}

void delay_us( uint32_t us )
{
    uint32_t const total_cycles = us * s_cycles_per_us;
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
        p_timer->start_tick  = delay_get_tick();
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
            result = ( ( delay_get_tick() - p_timer->start_tick )
                       >= p_timer->duration_ms );
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
        p_timer->start_tick = delay_get_tick();
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
        result = delay_get_tick() - p_timer->start_tick;
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
