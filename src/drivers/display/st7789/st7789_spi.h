/**
 * @file    st7789_spi.h
 * @brief   ST7789 hardware contract — SPI byte I/O and GPIO control.
 *
 *          Called by drivers/display/st7789/st7789.c.
 *          Implemented by BSP.
 */

#ifndef DRIVERS_DISPLAY_ST7789_SPI_H
#define DRIVERS_DISPLAY_ST7789_SPI_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"

/**
 * Send one byte over SPI — blocking.
 */
result_t st7789_spi_write_byte( uint8_t data );

/**
 * Send a buffer of bytes over SPI — blocking.
 */
result_t st7789_spi_write_buf( uint8_t const * buf, uint32_t len );

/**
 * Set DC pin — true = data, false = command.
 */
void st7789_dc_set( bool data );

/**
 * Set CS pin — true = deselect (high), false = select (low).
 */
void st7789_cs_set( bool deselect );

/**
 * Set RST pin — true = release (high), false = assert reset (low).
 */
void st7789_rst_set( bool release );

/**
 * Set backlight — true = on, false = off.
 */
void st7789_bl_set( bool on );

/**
 * Delay in milliseconds — used during init sequence.
 */
void st7789_delay_ms( uint32_t ms );

#endif /* DRIVERS_DISPLAY_ST7789_SPI_H */
