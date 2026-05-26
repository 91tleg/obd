#include "bsp/nucleo_h753zi/uart.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/gpio.h"
#include "hal/rcc.h"
#include "hal/uart.h"
#include "hal/nvic.h"

static uart_driver_t s_debug;
static uint8_t s_rx_buf[ BSP_DEBUG_RX_BUF_SIZE ];
static uint8_t s_tx_buf[ BSP_DEBUG_TX_BUF_SIZE ];

void bsp_uart_debug_init( void )
{
    rcc_gpio_clk_enable( BSP_DEBUG_TX_PORT );
    rcc_gpio_clk_enable( BSP_DEBUG_RX_PORT );
    rcc_uart_clk_enable( BSP_DEBUG_UART );
    rcc_uart_reset( BSP_DEBUG_UART );

    gpio_set_mode( BSP_DEBUG_TX_PORT, BSP_DEBUG_TX_PIN, GPIO_MODE_AF );
    gpio_set_af( BSP_DEBUG_TX_PORT, BSP_DEBUG_TX_PIN, BSP_DEBUG_TX_AF );
    gpio_set_speed( BSP_DEBUG_TX_PORT, BSP_DEBUG_TX_PIN, GPIO_SPEED_HIGH );
    gpio_set_pull( BSP_DEBUG_TX_PORT, BSP_DEBUG_TX_PIN, GPIO_PULL_NONE );

    gpio_set_mode( BSP_DEBUG_RX_PORT, BSP_DEBUG_RX_PIN, GPIO_MODE_AF );
    gpio_set_af( BSP_DEBUG_RX_PORT, BSP_DEBUG_RX_PIN, BSP_DEBUG_RX_AF );
    gpio_set_speed( BSP_DEBUG_RX_PORT, BSP_DEBUG_RX_PIN, GPIO_SPEED_HIGH );
    gpio_set_pull( BSP_DEBUG_RX_PORT, BSP_DEBUG_RX_PIN, GPIO_PULL_NONE );

    uart_init( BSP_DEBUG_UART,
               rcc_get_usart28_clk_freq(),
               BSP_DEBUG_BAUD,
               UART_WORD_8B,
               UART_STOP_1,
               UART_PARITY_NONE,
               UART_OVER16,
               UART_HWCTL_NONE );

    uart_driver_init( &s_debug,
                      BSP_UART_CH_DEBUG,
                      s_rx_buf, BSP_DEBUG_RX_BUF_SIZE,
                      s_tx_buf, BSP_DEBUG_TX_BUF_SIZE );

    nvic_enable_irq( BSP_DEBUG_IRQn, BSP_DEBUG_IRQ_PRIORITY );
}

uart_driver_t * bsp_uart_debug( void )
{
    return &s_debug;
}

void USART3_IRQHandler( void )
{
    uart_driver_irq_handler( &s_debug );
}
