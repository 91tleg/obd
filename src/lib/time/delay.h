/**
 * @file    delay.h
 * @brief   Blocking and non-blocking delay utilities
 */

#ifndef LIB_TIME_DELAY_H
#define LIB_TIME_DELAY_H

#include <stdint.h>
#include <stdbool.h>

/* uses SystemCoreClock — call after system_init() sets the clock */
void delay_init( void );

/* called from sys_tick_handler every 1ms — do not call directly */
void delay_tick( void );

uint32_t delay_get_tick( void );

/*
 * blocking
 * use only where timing accuracy is critical
 */
void delay_ms( uint32_t ms );
void delay_us( uint32_t us );     /* spin loop */

/*
 * non-blocking timer
 * use for timeouts, polling intervals, debounce
 * never blocks — caller checks expired()
 */
typedef struct
{
    uint32_t start_tick;
    uint32_t duration_ms;
    bool running;
} delay_timer_t;

void delay_timer_start( delay_timer_t * p_timer, uint32_t duration_ms );
bool delay_timer_expired( delay_timer_t * p_timer );
void delay_timer_reset( delay_timer_t * p_timer );
void delay_timer_stop( delay_timer_t * p_timer );
uint32_t delay_timer_elapsed( delay_timer_t * p_timer );
bool delay_timer_running( const delay_timer_t * p_timer );

#endif /* LIB_TIME_DELAY_H */
