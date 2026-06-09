/**
 * @file    can/protocol.h
 * @brief   OBD-II over CAN (ISO 15765-4) protocol API
 */

#ifndef DRIVERS_OBD_CAN_PROTOCOL_H
#define DRIVERS_OBD_CAN_PROTOCOL_H

#include <stdint.h>
#include "lib/core/result.h"
#include "drivers/protocol/obd/obd_types.h"
#include "drivers/protocol/obd/can/can_tp.h"

typedef struct
{
    can_tp_ctx_t can_tp;
} obd_can_ctx_t;

/**
 * Initialize OBD over CAN.
 * Initializes the CAN TP. CAN peripheral and GPIO must be
 * configured by BSP before calling.
 */
result_t obd_can_init( obd_can_ctx_t * ctx );

/**
 * Query which PIDs the ECU supports.
 * Polls support groups 0x00, 0x20, 0x40, 0x60 in sequence.
 * Results stored internally — subsequent obd_can_poll() uses them.
 */
result_t obd_can_query_supported( obd_can_ctx_t * ctx );

/**
 * Manually set the PID list to poll.
 * Overrides the result of obd_can_query_supported().
 */
result_t obd_can_set_pids( obd_can_ctx_t * ctx,
                           uint8_t const * pids,
                           uint32_t count );

/**
 * Poll all configured PIDs and decode responses.
 */
result_t obd_can_poll( obd_can_ctx_t * ctx,
                       live_data_t * p_out,
                       uint32_t timestamp );

/**
 * Read stored (Mode 03) and pending (Mode 07) DTCs.
 * Results appended into p_out up to DTC_LIST_MAX.
 */
result_t obd_can_read_dtcs( obd_can_ctx_t * ctx,
                            dtc_list_t * p_out );

/**
 * Clear stored DTCs (Mode 04).
 */
result_t obd_can_clear_dtcs( obd_can_ctx_t * ctx );

#endif /* DRIVERS_OBD_CAN_PROTOCOL_H */
