/**
 * @file    app_obd.c
 * @brief   OBD-II application layer implementation
 *
 *          State machine:
 *
 *          DISCONNECTED
 *               │  obd_app_connect()
 *               ▼
 *          CONNECTING
 *               │  obd_protocol_init() OK
 *               ▼
 *          DISCOVERING
 *               │  obd_protocol_query_supported() OK
 *               ▼
 *          POLLING ◄──────────────────────────────────┐
 *               │                                      │
 *               │  dtc_pending                         │
 *               ▼                                      │
 *          DTC_READ ──── obd_protocol_read_dtcs() ─────┤
 *               │                                      │
 *               │  dtc_clear_pending                   │
 *               ▼                                      │
 *          DTC_CLEAR ─── obd_protocol_clear_dtcs() ───►┘
 *               │
 *               │  transport error
 *               ▼
 *          ERROR
 */

#include "app/app_obd.h"
#include <stddef.h>

static void set_state( obd_app_t * app, obd_state_t state )
{
    app->state = state;
}

static void handle_polling( obd_app_t * app )
{
    result_t result = RES_OK;

    /* keep-alive — must fire before P3_MAX on KWP, no-op on CAN */
    if( delay_timer_expired( &app->keepalive_timer ) )
    {
        result = obd_protocol_keep_alive( app->protocol );
        delay_timer_reset( &app->keepalive_timer );
    }

    if( RES_IS_FAILED( result ) )
    {
        set_state( app, OBD_STATE_ERROR );
    }
    else if( app->dtc_clear_pending )
    {
        app->dtc_clear_pending = false;
        set_state( app, OBD_STATE_DTC_CLEAR );
    }
    else if( app->dtc_pending )
    {
        app->dtc_pending = false;
        set_state( app, OBD_STATE_DTC_READ );
    }
    else if( delay_timer_expired( &app->poll_timer ) )
    {
        result = obd_protocol_poll( app->protocol,
                                    &app->live_data,
                                    delay_timer_elapsed( &app->poll_timer ) );
        delay_timer_reset( &app->poll_timer );

        if( ( RES_IS_FAILED( result ) ) && ( result != RES_ERR_NO_DATA ) )
        {
            set_state( app, OBD_STATE_ERROR );
        }
    }
    else
    {
        /* poll not yet due — nothing to do */
    }
}

static void handle_dtc_read( obd_app_t * app )
{
    result_t result;

    result = obd_protocol_read_dtcs( app->protocol, &app->dtcs );

    if( ( RES_IS_OK( result ) ) || ( result == RES_ERR_NO_DATA ) )
    {
        set_state( app, OBD_STATE_POLLING );
    }
    else
    {
        set_state( app, OBD_STATE_ERROR );
    }
}

static void handle_dtc_clear( obd_app_t * app )
{
    result_t result;

    result = obd_protocol_clear_dtcs( app->protocol );

    if( RES_IS_OK( result ) )
    {
        app->dtcs.count = 0U;
        set_state( app, OBD_STATE_POLLING );
    }
    else
    {
        set_state( app, OBD_STATE_ERROR );
    }
}

void obd_app_init( obd_app_t * app )
{
    if( app != NULL )
    {
        app->protocol          = NULL;
        app->state             = OBD_STATE_DISCONNECTED;
        app->dtc_pending       = false;
        app->dtc_clear_pending = false;
        app->live_data.count   = 0U;
        app->dtcs.count        = 0U;

        delay_timer_stop( &app->poll_timer      );
        delay_timer_stop( &app->keepalive_timer );
    }
}

result_t obd_app_connect( obd_app_t * app )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( app != NULL )
    {
        set_state( app, OBD_STATE_CONNECTING );

        result = obd_protocol_init( &app->protocol );

        if( RES_IS_OK( result ) )
        {
            set_state( app, OBD_STATE_DISCOVERING );

            result = obd_protocol_query_supported( app->protocol );

            if( RES_IS_OK( result ) )
            {
                delay_timer_start( &app->poll_timer,
                                   OBD_APP_POLL_INTERVAL_MS );
                delay_timer_start( &app->keepalive_timer,
                                   OBD_APP_KEEPALIVE_INTERVAL_MS );
                set_state( app, OBD_STATE_POLLING );
            }
            else
            {
                set_state( app, OBD_STATE_ERROR );
            }
        }
        else
        {
            set_state( app, OBD_STATE_ERROR );
        }
    }

    return result;
}

void obd_app_tick( obd_app_t * app )
{
    if( app != NULL )
    {
        switch( app->state )
        {
            case OBD_STATE_POLLING:
                handle_polling( app );
                break;

            case OBD_STATE_DTC_READ:
                handle_dtc_read( app );
                break;

            case OBD_STATE_DTC_CLEAR:
                handle_dtc_clear( app );
                break;

            case OBD_STATE_DISCONNECTED:
            case OBD_STATE_CONNECTING:
            case OBD_STATE_DISCOVERING:
            case OBD_STATE_ERROR:
                /* no tick action — caller handles these states */
                break;

            default:
                break;
        }
    }
}

void obd_app_request_dtc_read( obd_app_t * app )
{
    if( app != NULL )
    {
        app->dtc_pending = true;
    }
}

void obd_app_request_dtc_clear( obd_app_t * app )
{
    if( app != NULL )
    {
        app->dtc_clear_pending = true;
    }
}

live_data_t const * obd_app_get_live_data( const obd_app_t * app )
{
    live_data_t const * result = NULL;

    if( app != NULL )
    {
        result = &app->live_data;
    }

    return result;
}

dtc_list_t const * obd_app_get_dtcs( const obd_app_t * app )
{
    dtc_list_t const * result = NULL;

    if( app != NULL )
    {
        result = &app->dtcs;
    }

    return result;
}

obd_state_t obd_app_get_state( const obd_app_t * app )
{
    obd_state_t result = OBD_STATE_DISCONNECTED;

    if( app != NULL )
    {
        result = app->state;
    }

    return result;
}
