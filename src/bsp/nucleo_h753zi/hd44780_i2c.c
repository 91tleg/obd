/**
 * @file    hd44780_i2c.c
 * @brief   HD44780 I2C implementation for Nucleo-H753ZI
 *          Satisfies hd44780_i2c.h contract
 */

#include "drivers/display/hd44780_i2c.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/i2c.h"

result_t hd44780_i2c_write( uint8_t dev_addr,
                            uint8_t data,
                            uint32_t timeout_ms )
{
    return i2c_write_byte( BSP_LCD_I2C, dev_addr, data, timeout_ms );
}
