/**
 * @file    hd44780_i2c.h
 * @brief   I2C write contract for HD44780 driver
 */

#ifndef DRIVERS_DISPLAY_HD44780_I2C_H
#define DRIVERS_DISPLAY_HD44780_I2C_H

#include <stdint.h>
#include "lib/core/result.h"

/* BSP must provide exactly this function */
result_t hd44780_i2c_write( uint8_t dev_addr,
                            uint8_t data,
                            uint32_t timeout_ms );

#endif /* DRIVERS_DISPLAY_HD44780_I2C_H */
