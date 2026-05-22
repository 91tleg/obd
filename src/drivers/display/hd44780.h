/**
 * @file    hd44780.h
 * @brief   HD44780 LCD driver via PCF8574 I2C expander
 *          4-bit mode, 2 rows, 16 columns
 */

#ifndef DRIVERS_DISPLAY_HD44780_H
#define DRIVERS_DISPLAY_HD44780_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"

typedef enum
{
    HD44780_ROW_0 = 0U,
    HD44780_ROW_1 = 1U,
} hd44780_row_t;

#define HD44780_COLS  ( 16U )
#define HD44780_ROWS  ( 2U )

typedef struct
{
    uint8_t dev_addr;
    bool backlight;
    bool initialized;
} hd44780_t;

result_t hd44780_init( hd44780_t * p_dev, uint8_t dev_addr );
result_t hd44780_clear( hd44780_t * p_dev );
result_t hd44780_home( hd44780_t * p_dev );
result_t hd44780_display_on( hd44780_t * p_dev );
result_t hd44780_display_off( hd44780_t * p_dev );
result_t hd44780_backlight( hd44780_t * p_dev, bool on );
result_t hd44780_set_cursor( hd44780_t * p_dev, uint8_t col,
                             hd44780_row_t row );
result_t hd44780_write_char( hd44780_t * p_dev, char c );
result_t hd44780_write_str( hd44780_t * p_dev,
                            const char * p_str,
                            uint32_t max_len );

#endif /* DRIVERS_DISPLAY_HD44780_H */
