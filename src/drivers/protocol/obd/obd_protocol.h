/**
 * @file    obd_protocol.h
 * @brief   Transport-agnostic OBD-II protocol contract.
 */

#ifndef DRIVERS_OBD_PROTOCOL_H
#define DRIVERS_OBD_PROTOCOL_H

#include <stdint.h>
#include "lib/core/result.h"
#include "drivers/protocol/obd/obd_types.h"

typedef struct obd_protocol_ctx_t obd_protocol_ctx_t;

/**
 * Initialize the OBD protocol and underlying transport.
 * For KWP2000: runs init sequence.
 * For CAN: initializes CAN TP context, no bus init needed.
 *
 * @param ctx  Output — populated on RES_OK.
 * @return RES_OK, RES_ERR_TIMEOUT, or RES_ERR_PROTOCOL.
 */
result_t obd_protocol_init( obd_protocol_ctx_t ** ctx );

/**
 * Keep the session alive.
 * For KWP2000: sends TesterPresent. Must be called before KWP_P3_MAX_MS.
 * For CAN: nop — CAN OBD is stateless.
 */
result_t obd_protocol_keep_alive( obd_protocol_ctx_t * ctx );

/**
 * Query which PIDs the ECU supports.
 * Polls support groups 0x00, 0x20, 0x40, 0x60.
 * Results stored internally.
 */
result_t obd_protocol_query_supported( obd_protocol_ctx_t * ctx );

/**
 * Manually override the PID list to poll.
 * Use when supported PIDs are known in advance.
 */
result_t obd_protocol_set_pids( obd_protocol_ctx_t * ctx,
                                uint8_t const * pids,
                                uint32_t count );

/**
 * Poll all configured PIDs and decode responses.
 *
 * @param ctx        Protocol context.
 * @param p_out      Output live data set — populated on RES_OK.
 * @param timestamp  Current tick in ms — stamped onto each parameter.
 * @return RES_OK, RES_ERR_NO_DATA, or transport error.
 */
result_t obd_protocol_poll( obd_protocol_ctx_t * ctx,
                            live_data_t * p_out,
                            uint32_t timestamp );

/**
 * Read stored (Mode 03) and pending (Mode 07) DTCs.
 * Appended into p_out up to DTC_LIST_MAX entries.
 * @return RES_OK if any DTCs read, RES_ERR_NO_DATA if none.
 */
result_t obd_protocol_read_dtcs( obd_protocol_ctx_t * ctx,
                                 dtc_list_t * p_out );

/**
 * Clear stored DTCs (Mode 04).
 */
result_t obd_protocol_clear_dtcs( obd_protocol_ctx_t * ctx );

#endif /* DRIVERS_OBD_PROTOCOL_H */
