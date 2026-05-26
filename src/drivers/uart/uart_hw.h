/**
 * @file    uart_hw.h
 * @brief   UART driver hardware contract — per channel.
 *
 *          The UART driver calls these with a channel ID.
 *          BSP provides implementation that dispatches by channel.
 */

#ifndef DRIVERS_UART_HW_H
#define DRIVERS_UART_HW_H

#include <stdint.h>
#include <stdbool.h>
#include "drivers/uart/uart_types.h"

/**
 * Read one byte from hardware RX register if available.
 * Returns true if a byte was read.
 */
bool uart_hw_read_byte( uart_channel_t channel, uint8_t * p_data );

/**
 * Write one byte to hardware TX register if space available.
 * Returns true if byte was accepted.
 */
bool uart_hw_write_byte( uart_channel_t channel, uint8_t data );

/**
 * True if RX register has data.
 */
bool uart_hw_rxne( uart_channel_t channel );

/**
 * True if TX register is empty.
 */
bool uart_hw_txe( uart_channel_t channel );

/**
 * Enable TXE interrupt — called after data pushed to TX ring.
 */
void uart_hw_enable_txe_irq( uart_channel_t channel );

/**
 * Disable TXE interrupt — called when TX ring drains to empty.
 */
void uart_hw_disable_txe_irq( uart_channel_t channel );

/**
 * True if any error flags set (ORE, FE, NE).
 */
bool uart_hw_error( uart_channel_t channel );

/**
 * Clear all error flags.
 */
void uart_hw_clear_errors( uart_channel_t channel );

#endif /* DRIVERS_UART_HW_H */
