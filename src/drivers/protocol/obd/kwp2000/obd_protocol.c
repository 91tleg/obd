/**
 * @file    kwp2000/obd_protocol.c
 * @brief   KWP2000 implementation of the obd_protocol.h contract.
 */

#include "drivers/protocol/obd/obd_protocol.h"
#include <stddef.h>
#include "drivers/protocol/obd/kwp2000/protocol.h"

struct obd_protocol_ctx_t
{
    obd_kwp_ctx_t kwp;
};

result_t obd_protocol_init( obd_protocol_ctx_t ** ctx )
{
    result_t result = RES_ERR_INVALID_ARG;
    static struct obd_protocol_ctx_t s_ctx;

    if( ctx != NULL )
    {
        /* try fast init first, fall back to 5-baud */
        result = obd_kwp_init( &s_ctx.kwp, KWP_INIT_FAST );

        if( RES_IS_FAILED( result ) )
        {
            result = obd_kwp_init( &s_ctx.kwp, KWP_INIT_5BAUD );
        }

        if( RES_IS_OK( result ) )
        {
            *ctx = &s_ctx;
        }
    }

    return result;
}

result_t obd_protocol_keep_alive( obd_protocol_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_kwp_keep_alive( &ctx->kwp );
    }

    return result;
}

result_t obd_protocol_query_supported( obd_protocol_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_kwp_query_supported( &ctx->kwp );
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
        result = obd_kwp_set_pids( &ctx->kwp, pids, count );
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
        result = obd_kwp_poll( &ctx->kwp, p_out, timestamp );
    }

    return result;
}

result_t obd_protocol_read_dtcs( obd_protocol_ctx_t * ctx,
                                 dtc_list_t * p_out )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_kwp_read_dtcs( &ctx->kwp, p_out );
    }

    return result;
}

result_t obd_protocol_clear_dtcs( obd_protocol_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        result = obd_kwp_clear_dtcs( &ctx->kwp );
    }

    return result;
}
