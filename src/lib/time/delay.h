/**
 * @file    delay.h
 * @brief   Blocking and non-blocking delay utilities
 */

#ifndef LIB_TIME_DELAY_H
#define LIB_TIME_DELAY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t start_tick;
    uint32_t duration_ms;
    bool running;
} delay_timer_t;

/* blocking delays */
void delay_ms( uint32_t ms );
void delay_us( uint32_t us );

/* non-blocking timer */
void delay_timer_start( delay_timer_t * p_timer, uint32_t duration_ms );
bool delay_timer_expired( delay_timer_t * p_timer );
void delay_timer_reset( delay_timer_t * p_timer );
void delay_timer_stop( delay_timer_t * p_timer );
uint32_t delay_timer_elapsed( delay_timer_t * p_timer );
bool delay_timer_running( const delay_timer_t * p_timer );

#endif /* LIB_TIME_DELAY_H */
