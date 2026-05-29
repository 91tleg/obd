/**
 * @file    kwp2000/protocol.c
 * @brief   OBD-II over KWP2000 protocol implementation
 */

#include "drivers/protocol/obd/kwp2000/protocol.h"
#include <stddef.h>
#include "drivers/protocol/obd/kwp2000/kwp2000_timing.h"
#include "drivers/protocol/obd/kwp2000/kwp2000_hw.h"
#include "drivers/protocol/obd/cmd.h"
#include "drivers/protocol/obd/decoder.h"
#include "lib/time/delay_tick.h"
#include "lib/time/delay.h"
#include "lib/core/macros.h"

#define OBD_KWP_RESPONSE_TIMEOUT_MS  ( 200U )
#define OBD_MAX_PIDS                 ( 32U  )
#define OBD_RESP_OVERHEAD            ( 2U   )   /* mode echo + PID echo    */

/* KWP2000 OBD addressing */
#define OBD_KWP_TARGET   ( KWP_ADDR_FUNCTIONAL )   /* 0x33 broadcast       */
#define OBD_KWP_SOURCE   ( KWP_ADDR_TESTER     )   /* 0xF1 tester          */

/* TesterPresent service ID — ISO 14230-3 */
#define KWP_SID_TESTER_PRESENT  ( 0x3EU )

static uint8_t s_pids[ OBD_MAX_PIDS ];
static uint32_t s_pid_count = 0U;

static uint8_t const s_support_pids[] =
{
    OBD_PID_SUPPORTED_1,
    OBD_PID_SUPPORTED_2,
    OBD_PID_SUPPORTED_3,
    OBD_PID_SUPPORTED_4,
};

#define SUPPORT_PID_COUNT  ARRAY_SIZE( s_support_pids )

static bool is_support_pid( uint8_t pid )
{
    uint32_t i = 0U;
    bool found = false;

    while( ( i < SUPPORT_PID_COUNT ) && !found )
    {
        found = ( s_support_pids[ i ] == pid );
        ++i;
    }

    return found;
}

/**
 * KWP2000 requires at least P3_MIN between end of ECU response and
 * start of next tester request. Enforced before every transmission.
 */
static void enforce_p3( obd_kwp_ctx_t const * ctx )
{
    uint32_t elapsed = delay_get_tick() - ctx->last_tx_tick;

    if( elapsed < KWP_P3_MIN_MS )
    {
        delay_ms( KWP_P3_MIN_MS - elapsed );
    }
}

/**
 * Send one OBD request frame and receive one response frame.
 * Strips frame header — returns payload bytes only.
 */
static result_t transact( obd_kwp_ctx_t * ctx,
                          uint8_t const * req,
                          uint8_t req_len,
                          uint8_t * resp,
                          uint32_t * resp_len )
{
    result_t result;
    kwp_frame_t tx_frame;
    kwp_frame_t rx_frame;
    uint32_t i = 0U;

    enforce_p3( ctx );

    /* build request frame */
    tx_frame.target = OBD_KWP_TARGET;
    tx_frame.source = OBD_KWP_SOURCE;
    tx_frame.len    = req_len;

    while( i < ( uint32_t )req_len )
    {
        tx_frame.data[ i ] = req[ i ];
        ++i;
    }

    kwp_hw_flush_rx();

    result = kwp_send_frame( &tx_frame );

    ctx->last_tx_tick = delay_get_tick();

    if( RES_IS_OK( result ) )
    {
        result = kwp_recv_frame( &rx_frame, OBD_KWP_RESPONSE_TIMEOUT_MS );
    }

    if( RES_IS_OK( result ) )
    {
        /* copy payload out — strip KWP frame header */
        *resp_len = ( uint32_t )rx_frame.len;
        i = 0U;

        while( i < *resp_len )
        {
            resp[ i ] = rx_frame.data[ i ];
            ++i;
        }
    }

    return result;
}

result_t obd_kwp_init( obd_kwp_ctx_t * ctx,
                       kwp_init_method_t method )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        ctx->last_tx_tick   = 0U;
        ctx->session_active = false;
        s_pid_count         = 0U;

        if( method == KWP_INIT_FAST )
        {
            result = kwp_fast_init();
        }
        else
        {
            result = kwp_5baud_init( KWP_ADDR_ECU );
        }

        if( RES_IS_OK( result ) )
        {
            ctx->session_active = true;
            ctx->last_tx_tick   = delay_get_tick();
        }
    }

    return result;
}

result_t obd_kwp_keep_alive( obd_kwp_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;
    uint8_t req[ 1U ];
    uint32_t resp_len = 0U;

    if( ctx != NULL )
    {
        uint8_t resp[ KWP_MAX_DATA ];
        req[ 0U ] = KWP_SID_TESTER_PRESENT;
        result = transact( ctx, req, 1U, resp, &resp_len );
    }

    return result;
}

result_t obd_kwp_query_supported( obd_kwp_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        uint8_t cmd[ 2U ];
        uint32_t group = 0U;
        bool more = true;

        s_pid_count = 0U;
        result = RES_OK;

        while( ( RES_IS_OK( result ) ) &&
               ( group < SUPPORT_PID_COUNT ) &&
               ( more ) )
        {
            uint8_t resp[ KWP_MAX_DATA ];
            uint32_t resp_len = 0U;
            uint8_t sp = s_support_pids[ group ];
            uint8_t batch[ 32U ];
            uint32_t count = 0U;

            ( void )obd_cmd_current_data( sp, cmd );

            result = transact( ctx, cmd, 2U, resp, &resp_len );

            if( ( RES_IS_OK( result ) ) &&
                ( resp_len >= ( ( uint32_t )OBD_RESP_OVERHEAD + 4U ) ) &&
                ( resp[ 0U ] == ( OBD_MODE_CURRENT_DATA | 0x40U ) ) &&
                ( resp[ 1U ] == sp ) )
            {
                result = obd_decode_supported_pids( &resp[ 2U ], sp,
                                                     batch, 32U,
                                                     &count );

                if( RES_IS_OK( result ) )
                {
                    uint32_t i = 0U;

                    while( ( i < count ) &&
                           ( s_pid_count < OBD_MAX_PIDS ) )
                    {
                        if( !is_support_pid( batch[ i ] ) )
                        {
                            s_pids[ s_pid_count ] = batch[ i ];
                            ++s_pid_count;
                        }
                        ++i;
                    }

                    if( ( group + 1U ) < SUPPORT_PID_COUNT )
                    {
                        uint8_t next_sp = s_support_pids[ group + 1U ];
                        uint32_t j = 0U;

                        more = false;

                        while( ( j < count ) && !more )
                        {
                            more = ( batch[ j ] == next_sp );
                            ++j;
                        }
                    }
                    else
                    {
                        more = false;
                    }
                }
            }
            else
            {
                more = false;
            }

            ++group;
        }
    }

    return result;
}

result_t obd_kwp_set_pids( obd_kwp_ctx_t const * ctx,
                           uint8_t const * pids,
                           uint32_t count )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( pids != NULL ) &&
        ( count > 0U ) && ( count <= OBD_MAX_PIDS ) )
    {
        uint32_t i = 0U;

        while( i < count )
        {
            s_pids[ i ] = pids[ i ];
            ++i;
        }

        s_pid_count = count;
        result = RES_OK;
    }

    return result;
}

result_t obd_kwp_poll( obd_kwp_ctx_t * ctx,
                       live_data_t * p_out,
                       uint32_t timestamp )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( p_out != NULL ) )
    {
        if( s_pid_count == 0U )
        {
            result = RES_ERR_NO_DATA;
        }
        else
        {
            uint8_t cmd[ 2U ];
            uint8_t resp[ KWP_MAX_DATA ];
            uint32_t resp_len = 0U;
            uint32_t i = 0U;

            p_out->count = 0U;
            result = RES_OK;

            while( ( i < s_pid_count ) && ( RES_IS_OK( result ) ) )
            {
                ( void )obd_cmd_current_data( s_pids[ i ], cmd );

                result = transact( ctx, cmd, 2U, resp, &resp_len );

                if( ( RES_IS_OK( result ) ) &&
                    ( resp_len > ( uint32_t )OBD_RESP_OVERHEAD ) &&
                    ( resp[ 0U ] == ( OBD_MODE_CURRENT_DATA | 0x40U ) ) &&
                    ( resp[ 1U ] == s_pids[ i ] ) )
                {
                    result = obd_decode_pid(
                                 s_pids[ i ],
                                 &resp[ 2U ],
                                 ( uint8_t )( resp_len - OBD_RESP_OVERHEAD ),
                                 &p_out->params[ p_out->count ],
                                 timestamp );

                    if( RES_IS_OK( result ) )
                    {
                        p_out->params[ p_out->count ].valid = true;
                        p_out->count++;
                    }
                    else if( result == RES_ERR_UNSUPPORTED )
                    {
                        result = RES_OK;   /* unknown PID — skip, not fatal */
                    }
                    else
                    {
                        /* decode error — propagate */
                    }
                }
                else if( RES_IS_OK( result ) )
                {
                    /* malformed response — skip PID */
                }
                else
                {
                    /* transport error — propagate */
                }

                ++i;
            }
        }
    }

    return result;
}

result_t obd_kwp_read_dtcs( obd_kwp_ctx_t * ctx,
                            dtc_list_t * p_out )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( p_out != NULL ) )
    {
        uint8_t cmd[ 1U ];
        uint8_t resp[ KWP_MAX_DATA ];
        uint32_t resp_len = 0U;

        p_out->count = 0U;

        /* stored DTCs — Mode 03 */
        ( void )obd_cmd_read_dtc( cmd );
        result = transact( ctx, cmd, 1U, resp, &resp_len );

        if( ( RES_IS_OK( result ) ) && ( resp_len > 0U ) &&
            ( resp[ 0U ] == ( OBD_MODE_READ_DTC | 0x40U ) ) )
        {
            ( void )obd_decode_dtcs( &resp[ 1 ],
                                     resp_len - 1U,
                                     false,
                                     p_out );
        }

        /* pending DTCs — Mode 07, append to same list */
        ( void )obd_cmd_pending_dtc( cmd );
        result = transact( ctx, cmd, 1U, resp, &resp_len );

        if( ( RES_IS_OK( result ) ) && ( resp_len > 0U ) &&
            ( resp[ 0U ] == ( OBD_MODE_PENDING_DTC | 0x40U ) ) )
        {
            ( void )obd_decode_dtcs( &resp[ 1 ],
                                     resp_len - 1U,
                                     true,
                                     p_out );
        }

        result = ( p_out->count > 0U ) ? RES_OK : RES_ERR_NO_DATA;
    }

    return result;
}

result_t obd_kwp_clear_dtcs( obd_kwp_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        uint8_t cmd[ 1U ];
        uint8_t resp[ KWP_MAX_DATA ];
        uint32_t resp_len = 0U;

        ( void )obd_cmd_clear_dtc( cmd );
        result = transact( ctx, cmd, 1U, resp, &resp_len );
    }

    return result;
}
