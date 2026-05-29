/**
 * @file    kwp2000_hw.h
 * @brief   KWP2000 hardware contract — K-Line physical layer.
 */

#ifndef DRIVERS_OBD_KWP2000_HW_H
#define DRIVERS_OBD_KWP2000_HW_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"

/**
 * Transmit one byte on the K-Line.
 * Blocks until the byte is accepted into the TX buffer.
 * Returns RES_OK or RES_ERR_TIMEOUT.
 */
result_t kwp_hw_send_byte( uint8_t data );

/**
 * Receive one byte from the K-Line.
 * Blocks until a byte arrives or timeout_ms elapses.
 * Returns RES_OK or RES_ERR_TIMEOUT.
 */
result_t kwp_hw_recv_byte( uint8_t * p_data, uint32_t timeout_ms );

/**
 * True if at least one byte is waiting in the RX buffer.
 */
bool kwp_hw_rx_available( void );

/**
 * Discard all bytes currently in the RX buffer.
 * Called before sending a request to clear any stale ECU bytes.
 */
void kwp_hw_flush_rx( void );

/**
 * Switch K-Line TX pin from UART AF to GPIO output mode.
 * Called before manual bit-banging during 5-baud init.
 */
void kwp_hw_bus_takeover( void );

/**
 * Switch K-Line TX pin back from GPIO output to UART AF mode.
 * Called after 5-baud init completes, before normal communication.
 */
void kwp_hw_bus_release( void );

/**
 * Drive K-Line low (dominant / 0V).
 * Only valid after kwp_hw_bus_takeover().
 */
void kwp_hw_line_low( void );

/**
 * Drive K-Line high (recessive / 12V via pull-up).
 * Only valid after kwp_hw_bus_takeover().
 */
void kwp_hw_line_high( void );

/**
 * Reconfigure the K-Line UART to the given baud rate.
 * Called after 5-baud init to switch from 5 baud to the
 * negotiated communication baud rate (typically 10400).
 * Returns RES_OK or RES_ERR_INVALID_ARG.
 */
result_t kwp_hw_set_baud( uint32_t baud );

#endif /* DRIVERS_OBD_KWP2000_HW_H */
