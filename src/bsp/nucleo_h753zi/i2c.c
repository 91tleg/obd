#include "bsp/nucleo_h753zi/i2c.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/i2c.h"
#include "hal/gpio.h"
#include "hal/rcc.h"

#define LCD_I2C_TIMING  ( 0x00C0EAFFU )

result_t bsp_i2c_init( void )
{
    rcc_gpio_clk_enable( BSP_LCD_SCL_PORT );
    rcc_gpio_clk_enable( BSP_LCD_SDA_PORT );
    rcc_i2c_clk_enable( BSP_LCD_I2C );

    gpio_set_mode( BSP_LCD_SCL_PORT, BSP_LCD_SCL_PIN, GPIO_MODE_AF );
    gpio_set_af( BSP_LCD_SCL_PORT, BSP_LCD_SCL_PIN, BSP_LCD_SCL_AF );
    gpio_set_otype( BSP_LCD_SCL_PORT, BSP_LCD_SCL_PIN, GPIO_OTYPE_OPEN_DRAIN );
    gpio_set_pull( BSP_LCD_SCL_PORT, BSP_LCD_SCL_PIN, GPIO_PULL_NONE );
    gpio_set_speed( BSP_LCD_SCL_PORT, BSP_LCD_SCL_PIN, GPIO_SPEED_LOW );

    gpio_set_mode( BSP_LCD_SDA_PORT, BSP_LCD_SDA_PIN, GPIO_MODE_AF );
    gpio_set_af( BSP_LCD_SDA_PORT, BSP_LCD_SDA_PIN, BSP_LCD_SDA_AF );
    gpio_set_otype( BSP_LCD_SDA_PORT, BSP_LCD_SDA_PIN, GPIO_OTYPE_OPEN_DRAIN );
    gpio_set_pull( BSP_LCD_SDA_PORT, BSP_LCD_SDA_PIN, GPIO_PULL_NONE );
    gpio_set_speed( BSP_LCD_SDA_PORT, BSP_LCD_SDA_PIN, GPIO_SPEED_LOW );

    return i2c_init( BSP_LCD_I2C, LCD_I2C_TIMING );
}
