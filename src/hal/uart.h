#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cmsis/stm32h753xx.h"
#include "lib/core/result.h"

typedef enum
{
    UART_WORD_8B       = 0U,
    UART_WORD_9B       = 1U,
    UART_WORD_7B       = 2U,
} uart_word_len_t;

typedef enum
{
    UART_STOP_1        = 0U,
    UART_STOP_0_5      = 1U,
    UART_STOP_2        = 2U,
    UART_STOP_1_5      = 3U,
} uart_stop_t;

typedef enum
{
    UART_PARITY_NONE   = 0U,
    UART_PARITY_EVEN   = 2U,
    UART_PARITY_ODD    = 3U,
} uart_parity_t;

typedef enum
{
    UART_OVER16        = 0U,
    UART_OVER8         = 1U,
} uart_oversampling_t;

typedef enum
{
    UART_HWCTL_NONE    = 0U,
    UART_HWCTL_RTS     = 1U,
    UART_HWCTL_CTS     = 2U,
    UART_HWCTL_RTS_CTS = 3U,
} uart_hwctl_t;

static inline void uart_enable( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 |= USART_CR1_UE;
    }
}

static inline void uart_tx_enable( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 |= USART_CR1_TE;
    }
}

static inline void uart_tx_disable( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 &= ~USART_CR1_TE;
    }
}

static inline void uart_rx_enable( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 |= USART_CR1_RE;
    }
}

static inline void uart_rx_disable( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 &= ~USART_CR1_RE;
    }
}

static inline void uart_enable_irq_idle( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 |= USART_CR1_IDLEIE;
    }
}

static inline void uart_disable_irq_idle( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 &= ~USART_CR1_IDLEIE;
    }
}

static inline void uart_enable_irq_rxne( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 |= USART_CR1_RXNEIE_RXFNEIE;
    }
}

static inline void uart_disable_irq_rxne( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;
    }
}

static inline void uart_enable_irq_txe( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 |= USART_CR1_TXEIE_TXFNFIE;
    }
}

static inline void uart_disable_irq_txe( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
    }
}

static inline void uart_enable_irq_tc( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 |= USART_CR1_TCIE;
    }
}

static inline void uart_disable_irq_tc( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->CR1 &= ~USART_CR1_TCIE;
    }
}

static inline bool uart_flag_rxne( const USART_TypeDef * p_uart )
{
    bool result = false;

    if( p_uart != NULL )
    {
        result = ( ( p_uart->ISR & USART_ISR_RXNE_RXFNE ) != 0U );
    }

    return result;
}

static inline bool uart_flag_txe( const USART_TypeDef * p_uart )
{
    bool result = false;

    if( p_uart != NULL )
    {
        result = ( ( p_uart->ISR & USART_ISR_TXE_TXFNF ) != 0U );
    }

    return result;
}

static inline bool uart_flag_tc( const USART_TypeDef * p_uart )
{
    bool result = false;

    if( p_uart != NULL )
    {
        result = ( ( p_uart->ISR & USART_ISR_TC ) != 0U );
    }

    return result;
}

static inline bool uart_flag_ore( const USART_TypeDef * p_uart )
{
    bool result = false;

    if( p_uart != NULL )
    {
        result = ( ( p_uart->ISR & USART_ISR_ORE ) != 0U );
    }

    return result;
}

static inline bool uart_flag_fe( const USART_TypeDef * p_uart )
{
    bool result = false;

    if( p_uart != NULL )
    {
        result = ( ( p_uart->ISR & USART_ISR_FE ) != 0U );
    }

    return result;
}

static inline bool uart_flag_ne( const USART_TypeDef * p_uart )
{
    bool result = false;

    if( p_uart != NULL )
    {
        result = ( ( p_uart->ISR & USART_ISR_NE ) != 0U );
    }

    return result;
}

static inline bool uart_flag_idle( const USART_TypeDef * p_uart )
{
    bool result = false;

    if( p_uart != NULL )
    {
        result = ( ( p_uart->ISR & USART_ISR_IDLE ) != 0U );
    }

    return result;
}

static inline void uart_clear_ore( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->ICR = USART_ICR_ORECF;
    }
}

static inline void uart_clear_fe( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->ICR = USART_ICR_FECF;
    }
}

static inline void uart_clear_ne( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->ICR = USART_ICR_NECF;
    }
}

static inline void uart_clear_tc( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->ICR = USART_ICR_TCCF;
    }
}

static inline void uart_clear_idle( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->ICR = USART_ICR_IDLECF;
    }
}

static inline void uart_clear_errors( USART_TypeDef * p_uart )
{
    if( p_uart != NULL )
    {
        p_uart->ICR = USART_ICR_ORECF
                    | USART_ICR_FECF
                    | USART_ICR_NECF;
    }
}

static inline bool uart_try_write_byte( USART_TypeDef * p_uart, uint8_t data )
{
    bool result = false;

    if( ( p_uart != NULL ) &&
        ( ( p_uart->ISR & USART_ISR_TXE_TXFNF ) != 0U ) )
    {
        p_uart->TDR = ( uint32_t )data;
        result      = true;
    }

    return result;
}

static inline bool uart_try_read_byte( const USART_TypeDef * p_uart,
                                       uint8_t * p_data )
{
    bool result = false;

    if( ( p_uart != NULL ) && ( p_data != NULL ) &&
        ( ( p_uart->ISR & USART_ISR_RXNE_RXFNE ) != 0U ) )
    {
        *p_data = ( uint8_t )( p_uart->RDR & 0xFFU );
        result  = true;
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
                    uart_hwctl_t hwctl );

result_t uart_disable( USART_TypeDef * p_uart, uint32_t timeout_ms );

result_t uart_write_byte_blocking( USART_TypeDef * p_uart,
                                   uint8_t data,
                                   uint32_t timeout_ms );

result_t uart_read_byte_blocking( USART_TypeDef * p_uart,
                                  uint8_t * p_data,
                                  uint32_t timeout_ms );

#endif /* HAL_UART_H */
