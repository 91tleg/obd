/**
 * @file    kwp2000.h
 * @brief   ISO 14230 KWP2000 transport layer
 */

#ifndef DRIVERS_OBD_KWP2000_H
#define DRIVERS_OBD_KWP2000_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"
#include "drivers/protocol/obd/kwp2000/kwp2000_timing.h"

#define KWP_FMT_BYTE         ( 0x80U )  /* format byte base — physical addr */
#define KWP_HDR_SIZE         ( 3U )     /* fmt + tgt + src                  */
#define KWP_CS_SIZE          ( 1U )     /* single checksum byte             */
#define KWP_MAX_DATA         ( 255U )   /* max payload bytes per frame      */
#define KWP_MAX_FRAME        ( KWP_HDR_SIZE + KWP_MAX_DATA + KWP_CS_SIZE )

#define KWP_ADDR_ECU         ( 0x01U )  /* physical ECU address             */
#define KWP_ADDR_TESTER      ( 0xF1U )  /* tester source address            */
#define KWP_ADDR_FUNCTIONAL  ( 0x33U )  /* functional broadcast address     */

typedef enum
{
    KWP_INIT_FAST   = 0U,   /* ISO 14230-2 fast init                      */
    KWP_INIT_5BAUD  = 1U,   /* ISO 14230-2 5-baud init                    */
} kwp_init_method_t;

typedef struct
{
    uint8_t target;                /* destination address                  */
    uint8_t source;                /* source address                       */
    uint8_t data[ KWP_MAX_DATA ];  /* payload — service ID + parameters    */
    uint8_t len;                   /* payload length in bytes              */
} kwp_frame_t;

/**
 * @brief Fast init
 *
 * Sends a 25ms low / 25ms high pulse then waits for ECU sync byte.
 * Faster than 5-baud init (~100ms vs ~2.6s) but not supported by
 * all ECUs.
 *
 * @return RES_OK if ECU responded with valid sync,
 *         RES_ERR_TIMEOUT if no response,
 *         RES_ERR_PROTOCOL if sync byte invalid.
 */
result_t kwp_fast_init( void );

/**
 * @brief 5-baud init
 *
 * Transmits target_addr at 5 baud (200ms/bit) via GPIO bit-bang,
 * then reads sync byte, keyword 1, keyword 2, and sends inverted
 * keyword 2 acknowledgement.
 *
 * @param target_addr  ECU address byte to send at 5 baud.
 * @return RES_OK, RES_ERR_TIMEOUT, or RES_ERR_PROTOCOL.
 */
result_t kwp_5baud_init( uint8_t target_addr );

/**
 * Build and transmit one KWP2000 frame.
 * Enforces P4 inter-byte timing between bytes.
 *
 * @param frame  Frame to transmit — target, source, data, len.
 * @return RES_OK, RES_ERR_INVALID_ARG, or RES_ERR_TIMEOUT.
 */
result_t kwp_send_frame( kwp_frame_t const * frame );

/**
 * Receive one KWP2000 frame.
 * Reads header, validates length, reads data, validates checksum.
 * Uses P1 timeout for inter-byte gaps within the frame.
 *
 * @param frame       Output — populated on RES_OK.
 * @param timeout_ms  Time to wait for first byte (P2 window).
 * @return RES_OK, RES_ERR_INVALID_ARG, RES_ERR_TIMEOUT,
 *         or RES_ERR_CHECKSUM.
 */
result_t kwp_recv_frame( kwp_frame_t * frame, uint32_t timeout_ms );

#endif /* DRIVERS_OBD_KWP2000_H */
