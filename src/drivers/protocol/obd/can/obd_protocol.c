/**
 * @file    can/obd_protocol.c
 * @brief   CAN implementation of the obd_protocol.h contract.
 */

#include "drivers/protocol/obd/obd_protocol.h"
#include "drivers/protocol/obd/can/protocol.h"
#include "lib/string/str.h"
#include <stddef.h>

struct obd_protocol_ctx_t
{
    obd_can_ctx_t can;
};

result_t obd_protocol_init( obd_protocol_ctx_t ** ctx )
{
    static struct obd_protocol_ctx_t s_ctx;

    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        mem_set( &s_ctx, 0U, sizeof( s_ctx ) );
        result = obd_can_init( &s_ctx.can );

        if( result == RES_OK )
        {
            *ctx = &s_ctx;
        }
    }

    return result;
}

result_t obd_protocol_keep_alive( obd_protocol_ctx_t * ctx )
{
    ( void )ctx;
    return RES_OK;   /* nop — CAN OBD is stateless */
}

result_t obd_protocol_query_supported( obd_protocol_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_can_query_supported( &ctx->can );
    }

    return result;
}

result_t obd_protocol_set_pids( obd_protocol_ctx_t * ctx,
                                uint8_t const * pids,
                                uint32_t count )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_can_set_pids( &ctx->can, pids, count );
    }

    return result;
}

result_t obd_protocol_poll( obd_protocol_ctx_t * ctx,
                            live_data_t * p_out,
                            uint32_t timestamp )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_can_poll( &ctx->can, p_out, timestamp );
    }

    return result;
}

result_t obd_protocol_read_dtcs( obd_protocol_ctx_t * ctx,
                                 dtc_list_t * p_out )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_can_read_dtcs( &ctx->can, p_out );
    }

    return result;
}

result_t obd_protocol_clear_dtcs( obd_protocol_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_can_clear_dtcs( &ctx->can );
    }

    return result;
}
