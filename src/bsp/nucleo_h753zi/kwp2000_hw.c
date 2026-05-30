/**
 * @file    bsp_kwp2000_hw.c
 * @brief   BSP implementation of the KWP2000 hardware contract.
 */

#include "drivers/protocol/obd/kwp2000/kwp2000_hw.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "drivers/uart/uart.h"
#include "hal/rcc.h"
#include "hal/gpio.h"
#include "hal/uart.h"
#include "lib/time/delay.h"

extern uart_driver_t * bsp_uart_obd( void );

result_t kwp_hw_send_byte( uint8_t data )
{
    return uart_driver_write_byte( bsp_uart_obd(), data )
           ? RES_OK
           : RES_ERR_OVERFLOW;
}

result_t kwp_hw_recv_byte( uint8_t * p_data, uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_data != NULL )
    {
        delay_timer_t timer;

        delay_timer_start( &timer, timeout_ms );
        result = RES_ERR_TIMEOUT;

        uart_driver_t * drv = bsp_uart_obd();

        while( ( !delay_timer_expired( &timer ) ) &&
               ( RES_IS_FAILED( result ) ) )
        {
            if( uart_driver_read_byte( drv, p_data ) )
            {
                result = RES_OK;
            }
        }
    }

    return result;
}

bool kwp_hw_rx_available( void )
{
    return uart_driver_rx_available( bsp_uart_obd() );
}

void kwp_hw_flush_rx( void )
{
    uart_driver_flush_rx( bsp_uart_obd() );
}

void kwp_hw_bus_takeover( void )
{
    gpio_set_mode( BSP_OBD_TX_PORT, BSP_OBD_TX_PIN, GPIO_MODE_OUTPUT );
    gpio_set( BSP_OBD_TX_PORT, BSP_OBD_TX_PIN );
}

void kwp_hw_bus_release( void )
{
    gpio_set_af  ( BSP_OBD_TX_PORT, BSP_OBD_TX_PIN, BSP_OBD_TX_AF );
    gpio_set_mode( BSP_OBD_TX_PORT, BSP_OBD_TX_PIN, GPIO_MODE_AF );
}

void kwp_hw_line_low( void )
{
    gpio_clear( BSP_OBD_TX_PORT, BSP_OBD_TX_PIN );
}

void kwp_hw_line_high( void )
{
    gpio_set( BSP_OBD_TX_PORT, BSP_OBD_TX_PIN );
}

result_t kwp_hw_set_baud( uint32_t baud )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( baud > 0U )
    {
        result = uart_init( BSP_OBD_UART,
                            rcc_get_usart28_clk_freq(),
                            baud,
                            UART_WORD_8B,
                            UART_STOP_1,
                            UART_PARITY_NONE,
                            UART_OVER16,
                            UART_HWCTL_NONE );
    }

    return result;
}
