/**
 * @file    kwp2000/protocol.h
 * @brief   OBD-II over KWP2000 (ISO 14230-4) protocol API
 */

#ifndef DRIVERS_OBD_KWP2000_PROTOCOL_H
#define DRIVERS_OBD_KWP2000_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"
#include "drivers/protocol/obd/obd_types.h"
#include "drivers/protocol/obd/kwp2000/kwp2000.h"

typedef struct
{
    kwp_frame_t frame;           /* reused for send/recv             */
    uint32_t    last_tx_tick;    /* tick of last transmitted request */
    bool        session_active;  /* true after successful init       */
} obd_kwp_ctx_t;

/**
 * Initialize OBD over KWP2000.
 * Runs the physical init sequence (fast or 5-baud).
 * Must be called before any other function.
 *
 * @param ctx     Protocol context.
 * @param method  KWP_INIT_FAST or KWP_INIT_5BAUD.
 * @return RES_OK, RES_ERR_TIMEOUT, or RES_ERR_PROTOCOL.
 */
result_t obd_kwp_init( obd_kwp_ctx_t * ctx,
                       kwp_init_method_t method );

/**
 * Send TesterPresent to keep the session alive.
 * Must be called before P3_MAX (5000ms) elapses since last request.
 * Use KWP_KEEPALIVE_MS from kwp2000_timing.h as the call interval.
 *
 * @return RES_OK or RES_ERR_TIMEOUT.
 */
result_t obd_kwp_keep_alive( obd_kwp_ctx_t * ctx );

/**
 * Query which PIDs the ECU supports.
 * Polls support groups 0x00, 0x20, 0x40, 0x60 in sequence.
 * Results stored internally — subsequent obd_kwp_poll() uses them.
 */
result_t obd_kwp_query_supported( obd_kwp_ctx_t * ctx );

/**
 * Manually set the PID list to poll.
 * Overrides the result of obd_kwp_query_supported().
 */
result_t obd_kwp_set_pids( obd_kwp_ctx_t const * ctx,
                           uint8_t const * pids,
                           uint32_t count );

/**
 * Poll all configured PIDs and decode responses.
 *
 * @param ctx        Protocol context.
 * @param p_out      Output live data set.
 * @param timestamp  Current tick — stamped onto each decoded parameter.
 */
result_t obd_kwp_poll( obd_kwp_ctx_t * ctx,
                       live_data_t * p_out,
                       uint32_t timestamp );

/**
 * Read stored (Mode 03) and pending (Mode 07) DTCs.
 * Results appended into p_out up to DTC_LIST_MAX.
 */
result_t obd_kwp_read_dtcs( obd_kwp_ctx_t * ctx,
                            dtc_list_t * p_out );

/**
 * Clear stored DTCs (Mode 04).
 */
result_t obd_kwp_clear_dtcs( obd_kwp_ctx_t * ctx );

#endif /* DRIVERS_OBD_KWP2000_PROTOCOL_H */
