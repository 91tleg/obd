/**
 * @file    can/protocol.c
 * @brief   OBD-II over CAN protocol implementation
 */

#include "drivers/protocol/obd/can/protocol.h"
#include <stddef.h>
#include "drivers/protocol/obd/obd_protocol.h"
#include "drivers/protocol/obd/cmd.h"
#include "drivers/protocol/obd/decoder.h"
#include "lib/core/macros.h"

#define OBD_RESPONSE_TIMEOUT_MS  ( 200U )
#define OBD_MAX_PIDS             ( 32U  )
#define OBD_RESP_OVERHEAD        ( ( uint32_t ) 2U )
#define OBD_RESP_BUF_SIZE        ( 128U )

/* support group PIDs */
static uint8_t const s_support_pids[] =
{
    OBD_PID_SUPPORTED_1,
    OBD_PID_SUPPORTED_2,
    OBD_PID_SUPPORTED_3,
    OBD_PID_SUPPORTED_4,
};

#define SUPPORT_PID_COUNT  ARRAY_SIZE( s_support_pids )

typedef struct
{
    uint8_t  pids[ OBD_MAX_PIDS ];
    uint32_t pid_count;
} obd_can_priv_t;

static obd_can_priv_t s_priv;

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

static bool is_negative_response( uint8_t const * resp, uint32_t len )
{
    return ( ( len >= 3U ) && ( resp[ 0 ] == 0x7FU ) );
}

static result_t transact( obd_can_ctx_t * ctx,
                          uint8_t const * req,
                          uint8_t req_len,
                          uint8_t * resp,
                          uint32_t * resp_len )
{
    result_t result;

    result = can_tp_send( &ctx->can_tp,
                          CAN_TP_OBD_REQ_ID,
                          req,
                          req_len );

    if( RES_IS_OK( result ) )
    {
        result = can_tp_recv( &ctx->can_tp,
                              resp,
                              OBD_RESP_BUF_SIZE,
                              resp_len,
                              OBD_RESPONSE_TIMEOUT_MS );
    }

    if( RES_IS_OK( result ) &&
        is_negative_response( resp, *resp_len ) )
    {
        result = RES_ERR_UNSUPPORTED;
    }

    return result;
}

result_t obd_can_init( obd_can_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        s_priv.pid_count = 0U;
        result = can_tp_init( &ctx->can_tp );
    }

    return result;
}

result_t obd_can_query_supported( obd_can_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        uint8_t  cmd[ 2U ];
        uint8_t  resp[ OBD_RESP_BUF_SIZE ];
        uint32_t resp_len = 0U;
        uint32_t group    = 0U;
        bool     more     = true;

        s_priv.pid_count = 0U;
        result           = RES_OK;

        while( ( group < SUPPORT_PID_COUNT ) &&
               ( more ) && ( RES_IS_OK( result ) ) )
        {
            uint8_t sp = s_support_pids[ group ];
            uint8_t batch[ 32U ];
            uint32_t count = 0U;

            ( void )obd_cmd_current_data( sp, cmd );

            result = transact( ctx, cmd, 2U, resp, &resp_len );

            if( ( RES_IS_OK( result ) ) &&
                ( resp_len >= ( OBD_RESP_OVERHEAD + 4U ) ) &&
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
                           ( s_priv.pid_count < OBD_MAX_PIDS ) )
                    {
                        if( !is_support_pid( batch[ i ] ) )
                        {
                            s_priv.pids[ s_priv.pid_count ] = batch[ i ];
                            s_priv.pid_count++;
                        }
                        ++i;
                    }

                    /* check if next support group is in this batch */
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
                more = false;   /* ECU did not respond to this group */
            }

            ++group;
        }
    }

    return result;
}

result_t obd_can_set_pids( obd_can_ctx_t * ctx,
                           uint8_t const * pids,
                           uint32_t count )
{
    result_t result = RES_ERR_INVALID_ARG;
    ( void )ctx;

    if( ( pids != NULL ) && ( count > 0U ) && ( count <= OBD_MAX_PIDS ) )
    {
        uint32_t i = 0U;

        while( i < count )
        {
            s_priv.pids[ i ] = pids[ i ];
            ++i;
        }

        s_priv.pid_count = count;
        result = RES_OK;
    }

    return result;
}

result_t obd_can_poll( obd_can_ctx_t * ctx,
                       live_data_t * p_out,
                       uint32_t timestamp )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( p_out != NULL ) )
    {
        if( s_priv.pid_count == 0U )
        {
            result = RES_ERR_NO_DATA;
        }
        else
        {
            uint8_t cmd[ 2U ];
            uint8_t resp[ OBD_RESP_BUF_SIZE ];
            uint32_t resp_len = 0U;
            uint32_t i = 0U;

            p_out->count = 0U;
            result = RES_OK;

            while( ( i < s_priv.pid_count ) &&
                   ( p_out->count < ARRAY_SIZE( p_out->params ) ) &&
                   RES_IS_OK( result ) )
            {
                ( void )obd_cmd_current_data( s_priv.pids[ i ], cmd );

                result = transact( ctx, cmd, 2U, resp, &resp_len );

                if( ( RES_IS_OK( result ) ) &&
                    ( resp_len > OBD_RESP_OVERHEAD ) &&
                    ( resp[ 0U ] == ( OBD_MODE_CURRENT_DATA | 0x40U ) ) &&
                    ( resp[ 1U ] == s_priv.pids[ i ] ) )
                {
                    result = obd_decode_pid(
                                 s_priv.pids[ i ],
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
                        /* unknown PID */
                        result = RES_OK;
                    }
                    else
                    {
                        /* decode error */
                    }
                }
                else if( RES_IS_OK( result ) )
                {
                    /* malformed response — skip this PID */
                }
                else
                {
                    /* transport error */
                }

                ++i;
            }
        }
    }

    return result;
}

result_t obd_can_read_dtcs( obd_can_ctx_t * ctx,
                            dtc_list_t * p_out )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( p_out != NULL ) )
    {
        uint8_t cmd[ 1U ];
        uint8_t resp[ OBD_RESP_BUF_SIZE ];
        uint32_t resp_len = 0U;

        p_out->count = 0U;

        /* stored DTCs — Mode 03 */
        ( void )obd_cmd_read_dtc( cmd );
        result = transact( ctx, cmd, 1U, resp, &resp_len );

        if( ( RES_IS_OK( result ) ) && ( resp_len > 0U ) &&
            ( resp[ 0U ] == ( OBD_MODE_READ_DTC | 0x40U ) ) )
        {
            ( void )obd_decode_dtcs( &resp[ 1U ],
                                     resp_len - 1U,
                                     false,
                                     p_out );
        }

        ( void )obd_cmd_pending_dtc( cmd );
        result = transact( ctx, cmd, 1U, resp, &resp_len );

        if( ( RES_IS_OK( result ) ) && ( resp_len > 0U ) &&
            ( resp[ 0U ] == ( OBD_MODE_PENDING_DTC | 0x40U ) ) )
        {
            ( void )obd_decode_dtcs( &resp[ 1U ],
                                     resp_len - 1U,
                                     true,
                                     p_out );
        }

        /* return OK if any DTCs found */
        result = ( p_out->count > 0U ) ? RES_OK : RES_ERR_NO_DATA;
    }

    return result;
}

result_t obd_can_clear_dtcs( obd_can_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        uint8_t  cmd[ 1U ];
        uint8_t  resp[ OBD_RESP_BUF_SIZE ];
        uint32_t resp_len = 0U;

        ( void )obd_cmd_clear_dtc( cmd );

        result = transact( ctx,
                           cmd,
                           1U,
                           resp,
                           &resp_len );

        if( RES_IS_OK( result ) )
        {
            if( ( resp_len >= 1U ) &&
                ( resp[ 0U ] == ( OBD_MODE_CLEAR_DTC | 0x40U ) ) )
            {
                result = RES_OK;
            }
            else
            {
                result = RES_ERR_PROTOCOL;
            }
        }
    }

    return result;
}
