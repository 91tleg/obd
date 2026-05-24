#include "uart.h"
#include "lib/time/delay.h"

result_t uart_disable( USART_TypeDef * p_uart, uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_uart != NULL )
    {
        delay_timer_t timer;
        delay_timer_start( &timer, timeout_ms );

        while( !uart_flag_tc( p_uart ) &&
               !delay_timer_expired( &timer ) )
        {
            /* wait for current transmission to complete */
        }

        p_uart->CR1 &= ~USART_CR1_UE;

        result = uart_flag_tc( p_uart ) ? RES_OK : RES_ERR_TIMEOUT;
    }

    return result;
}

result_t uart_init( USART_TypeDef * p_uart,
                    uint32_t pclk_hz,
                    uint32_t baud,
                    uart_word_len_t word_len,
                    uart_stop_t stop,
                    uart_parity_t parity,
                    uart_oversampling_t over,
                    uart_hwctl_t hwctl )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_uart != NULL ) && ( pclk_hz > 0U ) && ( baud > 0U ) )
    {
        result = uart_disable( p_uart, 10U );

        if( RES_IS_OK( result ) )
        {
            uart_clear_errors( p_uart );
            uart_clear_tc( p_uart );

            /* oversampling first */
            if( over == UART_OVER8 )
            {
                p_uart->CR1 |= USART_CR1_OVER8;
            }
            else
            {
                p_uart->CR1 &= ~USART_CR1_OVER8;
            }

            /* FIFO mode */
            p_uart->CR1 |= USART_CR1_FIFOEN;

            /* baud rate */
            if( over == UART_OVER8 )
            {
                uint32_t const usartdiv = ( 2U * pclk_hz
                                          + ( baud / 2U ) ) / baud;
                p_uart->BRR = ( usartdiv & 0xFFF0U )
                            | ( ( usartdiv & 0xFU ) >> 1U );
            }
            else
            {
                p_uart->BRR = ( pclk_hz + ( baud / 2U ) ) / baud;
            }

            /* word length */
            p_uart->CR1 &= ~( USART_CR1_M1 | USART_CR1_M0 );

            switch( word_len )
            {
                case UART_WORD_8B:
                    break;
                case UART_WORD_9B:
                    p_uart->CR1 |= USART_CR1_M0;
                    break;
                case UART_WORD_7B:
                    p_uart->CR1 |= USART_CR1_M1;
                    break;
                default:
                    break;
            }

            /* stop bits */
            p_uart->CR2 &= ~USART_CR2_STOP;
            p_uart->CR2 |= ( ( uint32_t )stop << USART_CR2_STOP_Pos );

            /* parity */
            p_uart->CR1 &= ~( USART_CR1_PCE | USART_CR1_PS );

            switch( parity )
            {
                case UART_PARITY_NONE:
                    break;
                case UART_PARITY_EVEN:
                    p_uart->CR1 |= USART_CR1_PCE;
                    break;
                case UART_PARITY_ODD:
                    p_uart->CR1 |= ( USART_CR1_PCE | USART_CR1_PS );
                    break;
                default:
                    break;
            }

            /* hardware flow control */
            p_uart->CR3 &= ~( USART_CR3_RTSE | USART_CR3_CTSE );

            switch( hwctl )
            {
                case UART_HWCTL_NONE:
                    break;
                case UART_HWCTL_RTS:
                    p_uart->CR3 |= USART_CR3_RTSE;
                    break;
                case UART_HWCTL_CTS:
                    p_uart->CR3 |= USART_CR3_CTSE;
                    break;
                case UART_HWCTL_RTS_CTS:
                    p_uart->CR3 |= ( USART_CR3_RTSE | USART_CR3_CTSE );
                    break;
                default:
                    break;
            }

            uart_tx_enable( p_uart );
            uart_rx_enable( p_uart );
            uart_enable( p_uart );
        }
    }

    return result;
}

result_t uart_write_byte_blocking( USART_TypeDef * p_uart,
                                   uint8_t data,
                                   uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_uart != NULL )
    {
        delay_timer_t timer;
        delay_timer_start( &timer, timeout_ms );
        result = RES_ERR_TIMEOUT;

        while( !delay_timer_expired( &timer ) )
        {
            if( uart_try_write_byte( p_uart, data ) )
            {
                result = RES_OK;
                break;
            }
        }
    }

    return result;
}

result_t uart_read_byte_blocking( USART_TypeDef * p_uart,
                                  uint8_t * p_data,
                                  uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_uart != NULL ) && ( p_data != NULL ) )
    {
        delay_timer_t timer;
        delay_timer_start( &timer, timeout_ms );
        result = RES_ERR_TIMEOUT;

        while( !delay_timer_expired( &timer ) )
        {
            if( uart_try_read_byte( p_uart, p_data ) )
            {
                result = RES_OK;
                break;
            }
        }
    }

    return result;
}
