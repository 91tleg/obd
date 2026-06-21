/**
 * @file    st7789_spi.c
 * @brief   ST7789 SPI implementation for Nucleo-H753ZI
 *          Satisfies hd44780_i2c.h contract
 */

#include "drivers/display/st7789/st7789_spi.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "lib/time/delay.h"
#include "hal/spi.h"
#include "hal/gpio.h"

result_t st7789_spi_write_byte( uint8_t data )
{
    return spi_write_byte( BSP_TFT_SPI, data );
}

result_t st7789_spi_write_buf( uint8_t const * buf, uint32_t len )
{
    return spi_write_buf( BSP_TFT_SPI, buf, len );
}

void st7789_dc_set( bool data )
{
    if( data )
    {
        gpio_set( BSP_TFT_DC_PORT, BSP_TFT_DC_PIN );
    }
    else
    {
        gpio_clear( BSP_TFT_DC_PORT, BSP_TFT_DC_PIN );
    }
}

void st7789_cs_set( bool deselect )
{
    if( deselect )
    {
        ( void )spi_wait_idle( BSP_TFT_SPI );
        gpio_set( BSP_TFT_CS_PORT, BSP_TFT_CS_PIN );
    }
    else
    {
        gpio_clear( BSP_TFT_CS_PORT, BSP_TFT_CS_PIN );
    }
}

void st7789_rst_set( bool release )
{
    if( release )
    {
        gpio_set( BSP_TFT_RST_PORT, BSP_TFT_RST_PIN );
    }
    else
    {
        gpio_clear( BSP_TFT_RST_PORT, BSP_TFT_RST_PIN );
    }
}

void st7789_bl_set( bool on )
{
    if( on )
    {
        gpio_set( BSP_TFT_BL_PORT, BSP_TFT_BL_PIN );
    }
    else
    {
        gpio_clear( BSP_TFT_BL_PORT, BSP_TFT_BL_PIN );
    }
}

void st7789_delay_ms( uint32_t ms )
{
    delay_ms( ms );
}
