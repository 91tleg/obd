/**
 * @file    obd_app.h
 * @brief   OBD-II application layer
 *
 *          Owns the poll loop, PID selection, DTC handling, and
 *          keep-alive management. Transport agnostic — calls through
 *          obd_protocol.h contract.
 */

#ifndef APP_OBD_H
#define APP_OBD_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"
#include "lib/time/delay.h"
#include "drivers/protocol/obd/obd_types.h"
#include "drivers/protocol/obd/obd_protocol.h"

typedef enum
{
    OBD_STATE_DISCONNECTED = 0U,
    OBD_STATE_CONNECTING   = 1U,
    OBD_STATE_DISCOVERING  = 2U,
    OBD_STATE_POLLING      = 3U,
    OBD_STATE_DTC_READ     = 4U,
    OBD_STATE_DTC_CLEAR    = 5U,
    OBD_STATE_ERROR        = 6U,
} obd_state_t;

typedef struct
{
    obd_protocol_ctx_t * protocol;
    obd_state_t          state;
    delay_timer_t        poll_timer;
    delay_timer_t        keepalive_timer;
    live_data_t          live_data;
    dtc_list_t           dtcs;
    bool                 dtc_pending;
    bool                 dtc_clear_pending;
} obd_app_t;

#define OBD_APP_POLL_INTERVAL_MS       ( 100U  )
#define OBD_APP_KEEPALIVE_INTERVAL_MS  ( 4500U )  /* before KWP P3_MAX 5000ms */

/**
 * Initialize the OBD application context.
 * Does not communicate with the ECU — call obd_app_connect() for that.
 */
void obd_app_init( obd_app_t * app );

/**
 * Run the OBD init sequence and PID discovery.
 * Blocking — returns when connected and PIDs discovered, or on error.
 *
 * @return RES_OK, RES_ERR_TIMEOUT, or RES_ERR_PROTOCOL.
 */
result_t obd_app_connect( obd_app_t * app );

/**
 * Tick — call every millisecond from the main loop.
 * Drives the poll loop, keep-alive, and pending DTC operations.
 * Non-blocking between polls.
 */
void obd_app_tick( obd_app_t * app );

/**
 * Request a DTC read on the next tick.
 * Results available via obd_app_get_dtcs() after state returns to POLLING.
 */
void obd_app_request_dtc_read( obd_app_t * app );

/**
 * Request a DTC clear on the next tick.
 */
void obd_app_request_dtc_clear( obd_app_t * app );

/**
 * Get a pointer to the latest live data.
 * Valid after first successful poll.
 */
live_data_t const * obd_app_get_live_data( const obd_app_t * app );

/**
 * Get a pointer to the latest DTC list.
 * Valid after obd_app_request_dtc_read() completes.
 */
dtc_list_t const * obd_app_get_dtcs( const obd_app_t * app );

/**
 * Get current connection state.
 */
obd_state_t obd_app_get_state( const obd_app_t * app );

#endif /* APP_OBD_H */
