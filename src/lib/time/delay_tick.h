/**
 * @file  delay_tick.h
 * @brief delay tick contract.
 */
 
#ifndef LIB_TIME_DELAY_TICK_H
#define LIB_TIME_DELAY_TICK_H

#include <stdint.h>

/**
 * Return current tick count in milliseconds.
 */
uint32_t delay_get_tick( void );

#endif /* LIB_TIME_DELAY_TICK_H */
