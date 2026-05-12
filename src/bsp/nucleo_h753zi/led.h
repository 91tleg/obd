#ifndef BSP_NUCLEO_H753ZI_LED_H
#define BSP_NUCLEO_H753ZI_LED_H

#include "resources.h"
#include "hal/rcc.h"
#include "hal/gpio.h"

static inline void bsp_led_green_off( void )
{
    gpio_clear( BSP_LED_GREEN_PORT, BSP_LED_GREEN_PIN );
}

static inline void bsp_led_green_init( void )
{
    rcc_gpio_clk_enable( BSP_LED_GREEN_PORT );
    gpio_set_mode ( BSP_LED_GREEN_PORT, BSP_LED_GREEN_PIN, GPIO_MODE_OUTPUT );
    gpio_set_speed( BSP_LED_GREEN_PORT, BSP_LED_GREEN_PIN, GPIO_SPEED_LOW );
    gpio_set_pull ( BSP_LED_GREEN_PORT, BSP_LED_GREEN_PIN, GPIO_PULL_NONE );
    bsp_led_green_off();
}

static inline void bsp_led_green_on( void )
{
    gpio_set( BSP_LED_GREEN_PORT, BSP_LED_GREEN_PIN );
}

static inline void bsp_led_green_toggle( void )
{
    gpio_toggle( BSP_LED_GREEN_PORT, BSP_LED_GREEN_PIN );
}

#endif /* BSP_NUCLEO_H753ZI_LED_H */
