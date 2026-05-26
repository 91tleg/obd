/**
 * @file    uart_hw.c
 * @brief   BSP implementation of the UART driver hardware contract.
 *
 *          Satisfies uart_hw.h — dispatches by channel ID to the correct
 *          USART peripheral via hal/uart.h.
 */

#include "drivers/uart/uart_hw.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/uart.h"
#include "lib/core/macros.h"

static USART_TypeDef * const s_uart_map[] =
{
    USART3,   /* BSP_UART_CH_DEBUG = 0 */
    UART4,    /* BSP_UART_CH_OBD   = 1 */
};

#define UART_CHANNEL_COUNT  ARRAY_SIZE( s_uart_map )

static USART_TypeDef * get_uart( uart_channel_t channel )
{
    USART_TypeDef * result = NULL;

    if( ( uint32_t )channel < UART_CHANNEL_COUNT )
    {
        result = s_uart_map[ channel ];
    }

    return result;
}

bool uart_hw_read_byte( uart_channel_t channel, uint8_t * p_data )
{
    return uart_try_read_byte( get_uart( channel ), p_data );
}

bool uart_hw_write_byte( uart_channel_t channel, uint8_t data )
{
    return uart_try_write_byte( get_uart( channel ), data );
}

bool uart_hw_rxne( uart_channel_t channel )
{
    return uart_flag_rxne( get_uart( channel ) );
}

bool uart_hw_txe( uart_channel_t channel )
{
    return uart_flag_txe( get_uart( channel ) );
}

void uart_hw_enable_txe_irq( uart_channel_t channel )
{
    uart_enable_irq_txe( get_uart( channel ) );
}

void uart_hw_disable_txe_irq( uart_channel_t channel )
{
    uart_disable_irq_txe( get_uart( channel ) );
}

bool uart_hw_error( uart_channel_t channel )
{
    USART_TypeDef * p_uart = get_uart( channel );

    return ( uart_flag_ore( p_uart ) ||
             uart_flag_fe ( p_uart ) ||
             uart_flag_ne ( p_uart ) );
}

void uart_hw_clear_errors( uart_channel_t channel )
{
    uart_clear_errors( get_uart( channel ) );
}
