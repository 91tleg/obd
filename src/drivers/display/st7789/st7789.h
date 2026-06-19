/**
 * @file    st7789.h
 * @brief   ST7789 240x240 TFT driver
 *
 *          Coordinate:
 *            (0,0) = top-left
 *            x increases right, y increases down
 */

#ifndef DRIVERS_DISPLAY_ST7789_H
#define DRIVERS_DISPLAY_ST7789_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"

#define ST7789_WIDTH   ( 240U )
#define ST7789_HEIGHT  ( 240U )

#define ST7789_BLACK   ( 0x0000U )
#define ST7789_WHITE   ( 0xFFFFU )
#define ST7789_RED     ( 0xF800U )
#define ST7789_GREEN   ( 0x07E0U )
#define ST7789_BLUE    ( 0x001FU )
#define ST7789_YELLOW  ( 0xFFE0U )
#define ST7789_CYAN    ( 0x07FFU )

#define FONT7X10_W     ( 7U  )
#define FONT7X10_H     ( 10U )
#define FONT7X10_FIRST ( 0x20U )   /* space */
#define FONT7X10_LAST  ( 0x7EU )   /* ~ */

/**
 * Runs the full init sequence.
 */
result_t st7789_init( void );

/**
 * Fill the entire screen with one color.
 */
result_t st7789_fill( uint16_t color );

/**
 * Draw a single character at pixel position (x, y).
 * Uses Font7x10. Characters outside the screen are clipped.
 *
 * @param x      Left edge of character in pixels.
 * @param y      Top edge of character in pixels.
 * @param c      ASCII character to draw.
 * @param fg     Foreground color (RGB565).
 * @param bg     Background color (RGB565).
 */
result_t st7789_draw_char( uint16_t x,
                           uint16_t y,
                           char c,
                           uint16_t fg,
                           uint16_t bg );

/**
 * Draw a null-terminated string starting at pixel position (x, y).
 * Characters wrap to the next line when exceed width.
 *
 * @param x      Left edge of first character.
 * @param y      Top edge of first character.
 * @param str    Null-terminated string.
 * @param fg     Foreground color.
 * @param bg     Background color.
 */
result_t st7789_draw_str( uint16_t x,
                          uint16_t y,
                          char const * str,
                          uint16_t fg,
                          uint16_t bg );


#endif /* DRIVERS_DISPLAY_ST7789_H */
