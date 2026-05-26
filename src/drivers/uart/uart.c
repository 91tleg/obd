/**
 * @file    uart.c
 * @brief   Buffered interrupt-driven UART driver implementation.
 */

#include "drivers/uart/uart.h"
#include <stddef.h>
#include "drivers/uart/uart_hw.h"

void uart_driver_init( uart_driver_t * p_drv,
                       uart_channel_t channel,
                       uint8_t * p_rx_buf,
                       uint32_t rx_buf_size,
                       uint8_t * p_tx_buf,
                       uint32_t tx_buf_size )
{
    if( ( p_drv != NULL ) && ( p_rx_buf != NULL ) &&
        ( p_tx_buf != NULL ) && ( rx_buf_size > 0U ) &&
        ( tx_buf_size > 0U ) )
    {
        p_drv->channel           = channel;
        p_drv->rx_overflow_count = 0U;
        p_drv->error_count       = 0U;
        p_drv->initialized       = false;

        ring_buf_init( &p_drv->rx_ring, p_rx_buf, rx_buf_size );
        ring_buf_init( &p_drv->tx_ring, p_tx_buf, tx_buf_size );

        p_drv->initialized = true;
    }
}

void uart_driver_irq_handler( uart_driver_t * drv )
{
    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        uart_channel_t const ch = drv->channel;
        uint8_t byte = 0U;

        /* RX */
        if( uart_hw_rxne( ch ) )
        {
            if( uart_hw_read_byte( ch, &byte ) )
            {
                if( !ring_buf_push( &drv->rx_ring, byte ) )
                {
                    drv->rx_overflow_count++;
                }
            }
        }

        /* TX */
        if( uart_hw_txe( ch ) )
        {
            if( ring_buf_pop( &drv->tx_ring, &byte ) )
            {
                ( void )uart_hw_write_byte( ch, byte );
            }
            else
            {
                uart_hw_disable_txe_irq( ch );
            }
        }

        if( uart_hw_error( ch ) )
        {
            drv->error_count++;
            uart_hw_clear_errors( ch );
        }
    }
}

uint32_t uart_driver_write( uart_driver_t * drv,
                            uint8_t const * p_data,
                            uint32_t len )
{
    uint32_t written = 0U;

    if( ( drv != NULL ) && ( p_data != NULL ) &&
        ( len > 0U ) && ( drv->initialized ) )
    {
        uint32_t i = 0U;

        while( i < len )
        {
            if( ring_buf_push( &drv->tx_ring, p_data[i] ) )
            {
                ++written;
            }
            ++i;
        }

        if( written > 0U )
        {
            uart_hw_enable_txe_irq( drv->channel );
        }
    }

    return written;
}

bool uart_driver_write_byte( uart_driver_t * drv, uint8_t data )
{
    bool result = false;

    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        if( ring_buf_push( &drv->tx_ring, data ) )
        {
            uart_hw_enable_txe_irq( drv->channel );
            result = true;
        }
    }

    return result;
}

uint32_t uart_driver_read( uart_driver_t * drv,
                           uint8_t * p_buf,
                           uint32_t len )
{
    uint32_t count = 0U;

    if( ( drv != NULL ) && ( p_buf != NULL ) &&
        ( len > 0U ) && ( drv->initialized ) )
    {
        while( ( count < len ) &&
               ring_buf_pop( &drv->rx_ring, &p_buf[ count ] ) )
        {
            ++count;
        }
    }

    return count;
}

bool uart_driver_read_byte( uart_driver_t * drv, uint8_t * p_data )
{
    bool result = false;

    if( ( drv != NULL ) && ( p_data != NULL ) &&
        ( drv->initialized ) )
    {
        result = ring_buf_pop( &drv->rx_ring, p_data );
    }

    return result;
}

bool uart_driver_peek_byte( uart_driver_t * drv, uint8_t * p_data )
{
    bool result = false;

    if( ( drv != NULL ) && ( p_data != NULL ) &&
        ( drv->initialized ) )
    {
        result = ring_buf_peek( &drv->rx_ring, p_data );
    }

    return result;
}

bool uart_driver_rx_available( const uart_driver_t * drv )
{
    bool result = false;

    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        result = !ring_buf_empty( &drv->rx_ring );
    }

    return result;
}

bool uart_driver_tx_busy( const uart_driver_t * drv )
{
    bool result = false;

    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        result = !ring_buf_empty( &drv->tx_ring );
    }

    return result;
}

uint32_t uart_driver_rx_count( const uart_driver_t * drv )
{
    uint32_t count = 0U;

    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        count = ring_buf_used( &drv->rx_ring );
    }

    return count;
}

uint32_t uart_driver_tx_free( const uart_driver_t * drv )
{
    uint32_t free_space = 0U;

    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        free_space = ring_buf_free( &drv->tx_ring );
    }

    return free_space;
}

void uart_driver_flush_rx( uart_driver_t * drv )
{
    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        ring_buf_clear( &drv->rx_ring );
    }
}

uint32_t uart_driver_error_count( const uart_driver_t * drv )
{
    uint32_t count = 0U;

    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        count = drv->error_count;
    }

    return count;
}

uint32_t uart_driver_overflow_count( const uart_driver_t * drv )
{
    uint32_t count = 0U;

    if( ( drv != NULL ) && ( drv->initialized ) )
    {
        count = drv->rx_overflow_count;
    }

    return count;
}
