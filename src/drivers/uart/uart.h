/**
 * @file    uart.h
 * @brief   Buffered interrupt-driven UART driver
 *
 *          Single producer / single consumer.
 *          TX: app writes to ring buffer, ISR drains it.
 *          RX: ISR fills ring buffer, app reads from it.
 */

#ifndef DRIVERS_UART_H
#define DRIVERS_UART_H

#include <stdint.h>
#include <stdbool.h>
#include "drivers/uart/uart_types.h"
#include "lib/collections/ring_buffer/ring_buffer.h"

typedef struct
{
    uart_channel_t channel;
    ring_buf_t     rx_ring;
    ring_buf_t     tx_ring;
    uint32_t       rx_overflow_count;
    uint32_t       error_count;
    bool           initialized;
} uart_driver_t;

void uart_driver_init( uart_driver_t * p_drv,
                       uart_channel_t channel,
                       uint8_t * p_rx_buf,
                       uint32_t rx_buf_size,
                       uint8_t * p_tx_buf,
                       uint32_t tx_buf_size );

/**
 * Call from USARTx_IRQHandler — handles RX, TX, and errors.
 */
void uart_driver_irq_handler( uart_driver_t * drv );

uint32_t uart_driver_write( uart_driver_t * drv,
                            uint8_t * p_data,
                            uint32_t len );
bool uart_driver_write_byte( uart_driver_t * drv,
                             uint8_t data );

uint32_t uart_driver_read( uart_driver_t * drv,
                           uint8_t * p_buf,
                           uint32_t len );
bool uart_driver_read_byte( uart_driver_t * drv,
                            uint8_t * p_data );
bool uart_driver_peek_byte( uart_driver_t * drv,
                            uint8_t * p_data );

bool uart_driver_rx_available( const uart_driver_t * drv );
bool uart_driver_tx_busy( const uart_driver_t * drv );
uint32_t uart_driver_rx_count( const uart_driver_t * drv );
uint32_t uart_driver_tx_free( const uart_driver_t * drv );
void uart_driver_flush_rx( uart_driver_t * drv );
uint32_t uart_driver_error_count( const uart_driver_t * drv );
uint32_t uart_driver_overflow_count( const uart_driver_t * drv );

#endif /* DRIVERS_UART_H */
