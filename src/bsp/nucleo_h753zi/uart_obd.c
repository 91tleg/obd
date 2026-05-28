#include "bsp/nucleo_h753zi/uart.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/gpio.h"
#include "hal/rcc.h"
#include "hal/uart.h"
#include "hal/nvic.h"

#define OBD_RX_BUF_SIZE   ( 64U )
#define OBD_TX_BUF_SIZE   ( 64U )
#define OBD_BAUD          ( 10400U )
#define OBD_IRQ_PRIORITY  ( 5U )

static uart_driver_t s_obd;
static uint8_t s_rx_buf[ OBD_RX_BUF_SIZE ];
static uint8_t s_tx_buf[ OBD_TX_BUF_SIZE ];

result_t bsp_uart_obd_init( void )
{
    result_t result;

    rcc_gpio_clk_enable( BSP_OBD_TX_PORT );
    rcc_gpio_clk_enable( BSP_OBD_RX_PORT );
    rcc_uart_clk_enable( BSP_OBD_UART );
    rcc_uart_reset(      BSP_OBD_UART );

    gpio_set_mode(  BSP_OBD_TX_PORT, BSP_OBD_TX_PIN, GPIO_MODE_AF    );
    gpio_set_af(    BSP_OBD_TX_PORT, BSP_OBD_TX_PIN, BSP_OBD_TX_AF   );
    gpio_set_speed( BSP_OBD_TX_PORT, BSP_OBD_TX_PIN, GPIO_SPEED_LOW  );
    gpio_set_pull(  BSP_OBD_TX_PORT, BSP_OBD_TX_PIN, GPIO_PULL_NONE  );

    gpio_set_mode(  BSP_OBD_RX_PORT, BSP_OBD_RX_PIN, GPIO_MODE_AF    );
    gpio_set_af(    BSP_OBD_RX_PORT, BSP_OBD_RX_PIN, BSP_OBD_RX_AF   );
    gpio_set_speed( BSP_OBD_RX_PORT, BSP_OBD_RX_PIN, GPIO_SPEED_LOW  );
    gpio_set_pull(  BSP_OBD_RX_PORT, BSP_OBD_RX_PIN, GPIO_PULL_NONE  );

    result = uart_init( BSP_OBD_UART,
                        rcc_get_usart28_clk_freq(),
                        OBD_BAUD,
                        UART_WORD_8B,
                        UART_STOP_1,
                        UART_PARITY_NONE,
                        UART_OVER16,
                        UART_HWCTL_NONE );

    if( RES_IS_OK( result ) )
    {
        uart_driver_init( &s_obd,
                          BSP_UART_CH_OBD,
                          s_rx_buf, OBD_RX_BUF_SIZE,
                          s_tx_buf, OBD_TX_BUF_SIZE );

        nvic_enable_irq( BSP_OBD_IRQn, OBD_IRQ_PRIORITY );
    }

    return result;
}

uart_driver_t * bsp_uart_obd( void )
{
    return &s_obd;
}

void UART4_IRQHandler( void )
{
    uart_driver_irq_handler( &s_obd );
}
