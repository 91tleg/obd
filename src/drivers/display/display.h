#ifndef DRIVERS_DISPLAY_DISPLAY_H
#define DRIVERS_DISPLAY_DISPLAY_H

#include <stdint.h>
#include "lib/core/result.h"

/**
 * Initialize the display.
 * BSP must configure hardware before calling.
 */
result_t display_init( void );

/**
 * Clear the display.
 */
result_t display_clear( void );

/**
 * Write a string.
 */
result_t display_print( uint8_t row, char const * str );

#endif /* DRIVERS_DISPLAY_DISPLAY_H */
