/**
 * @file    can.h
 * @brief   STM32H753 FDCAN HAL.
 */

#ifndef HAL_CAN_H
#define HAL_CAN_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h753xx.h"
#include "lib/core/result.h"

#define CAN_FRAME_DATA_LEN  ( 8U )

typedef struct
{
    uint32_t id;
    uint8_t  data[ CAN_FRAME_DATA_LEN ];
    uint8_t  dlc;
    uint8_t  pad[ 3U ];
} can_frame_t;

/**
 * Opaque NBTP register value.
 * Always construct with CAN_NBTP() — never write raw literals.
 */
typedef uint32_t can_timing_t;

/**
 * Build an NBTP register value.
 * All parameters are the human values — macro subtracts 1 internally.
 *
 * @param sjw    Sync jump width      (1–128)
 * @param brp    Baud rate prescaler  (1–512)
 * @param tseg1  Time segment 1       (2–256, prop + phase1)
 * @param tseg2  Time segment 2       (2–128, phase2)
 */
#define CAN_NBTP( sjw, brp, tseg1, tseg2 )                           \
    ( ( ( uint32_t )( (sjw)   - 1U ) << FDCAN_NBTP_NSJW_Pos   ) |   \
      ( ( uint32_t )( (brp)   - 1U ) << FDCAN_NBTP_NBRP_Pos   ) |   \
      ( ( uint32_t )( (tseg1) - 1U ) << FDCAN_NBTP_NTSEG1_Pos ) |   \
      ( ( uint32_t )( (tseg2) - 1U ) << FDCAN_NBTP_NTSEG2_Pos ) )

typedef struct
{
    uint32_t id_low;    /**< Lower bound of accepted standard ID (inclusive). */
    uint32_t id_high;   /**< Upper bound of accepted standard ID (inclusive). */
} can_filter_t;

/**
 * Initialize an FDCAN peripheral.
 *
 * Clock and GPIO must be configured by the BSP before calling this function.
 * Configures bit timing, message RAM layout (1 std filter, 8-element RX FIFO0,
 * 8-element TX FIFO), a single range filter for RX FIFO0, and rejects all
 * frames that do not match.
 *
 * @param p_can   Pointer to FDCAN1 or FDCAN2 peripheral.
 * @param timing  NBTP register value — construct with CAN_NBTP().
 * @param filter  Standard ID range filter (id_low..id_high, inclusive).
 * @return        RES_OK, RES_ERR_INVALID_ARG, or RES_ERR_TIMEOUT.
 */
result_t can_init( FDCAN_GlobalTypeDef * p_can,
                   can_timing_t timing,
                   can_filter_t * filter );

/**
 * Queue a frame for transmission via TX FIFO.
 *
 * Non-blocking. Returns RES_ERR_BUSY immediately if the TX FIFO is full.
 * frame->dlc must be <= 8.
 *
 * @param p_can   Pointer to FDCAN1 or FDCAN2 peripheral.
 * @param frame   Frame to transmit (not modified).
 * @return        RES_OK, RES_ERR_INVALID_ARG, or RES_ERR_BUSY.
 */
result_t can_send( FDCAN_GlobalTypeDef * p_can,
                   can_frame_t * frame );

/**
 * Receive a frame from RX FIFO0.
 *
 * Blocks until a frame arrives or @p timeout_ms elapses.
 * Checks bus-off status before waiting; returns RES_ERR_BUS immediately
 * if the peripheral is bus-off.
 *
 * @param p_can       Pointer to FDCAN1 or FDCAN2 peripheral.
 * @param frame       Output frame buffer (populated on RES_OK).
 * @param timeout_ms  Maximum time to wait in milliseconds.
 * @return            RES_OK, RES_ERR_INVALID_ARG, RES_ERR_TIMEOUT, or RES_ERR_BUS.
 */
result_t can_recv( FDCAN_GlobalTypeDef * p_can,
                   can_frame_t * frame,
                   uint32_t timeout_ms );

#endif /* HAL_CAN_H */
