/**
 * @file    can_stub.c
 * @brief   CAN HAL stub implementation for SIL testing.
 */

#include "can_stub.h"
#include <string.h>

static can_frame_t s_sent[ STUB_MAX_FRAMES ];
static uint32_t s_sent_count;

static can_frame_t s_recv_queue[ STUB_MAX_FRAMES ];
static uint32_t s_recv_head;
static uint32_t s_recv_tail;
static uint32_t s_recv_calls;
static result_t s_recv_error;
static uint32_t s_errors;
static void ( *s_on_send )( can_frame_t const * ) = NULL;

void stub_reset( void )
{
    memset( s_sent, 0, sizeof( s_sent ) );
    memset( s_recv_queue, 0, sizeof( s_recv_queue ) );
    s_sent_count = 0U;
    s_recv_head  = 0U;
    s_recv_tail  = 0U;
    s_recv_calls = 0U;
    s_recv_error = RES_OK;
    s_errors     = 0U;
    s_on_send    = NULL;
}

void stub_queue_recv( can_frame_t const * frame )
{
    if( frame == NULL )
    {
        return;
    }

    if( s_recv_tail < STUB_MAX_FRAMES )
    {
        s_recv_queue[ s_recv_tail++ ] = *frame;
    }
    else
    {
        s_errors |= STUB_ERR_RECV_OVERFLOW;
    }
}

void stub_set_recv_error( result_t err )
{
    s_recv_error = err;
}

uint32_t stub_sent_count( void )
{
    return s_sent_count;
}

can_frame_t const * stub_sent( uint32_t index )
{
    if( index < s_sent_count )
    {
        return &s_sent[ index ];
    }
    return NULL;
}

uint32_t stub_recv_remaining( void )
{
    return s_recv_tail - s_recv_head;
}

uint32_t stub_recv_calls( void )
{
    return s_recv_calls;
}

void stub_set_on_send( void ( *cb )( can_frame_t const * ) )
{
    s_on_send = cb;
}

uint32_t stub_errors( void )
{
    return s_errors;
}

result_t can_send( FDCAN_GlobalTypeDef * p_can, can_frame_t const * frame )
{
    ( void )p_can;

    if( frame == NULL )
    {
        return RES_ERR_INVALID_ARG;
    }

    if( s_sent_count < STUB_MAX_FRAMES )
    {
        s_sent[ s_sent_count++ ] = *frame;
    }
    else
    {
        s_errors |= STUB_ERR_SENT_OVERFLOW;
    }

    if( s_on_send != NULL )
    {
        s_on_send( frame );
    }
    return RES_OK;
}

result_t can_recv( FDCAN_GlobalTypeDef * p_can,
                   can_frame_t * frame,
                   uint32_t timeout_ms )
{
    ( void )p_can;
    ( void )timeout_ms;

    ++s_recv_calls;

    if( frame == NULL )
    {
        return RES_ERR_INVALID_ARG;
    }

    if( s_recv_error != RES_OK )
    {
        result_t err = s_recv_error;
        s_recv_error = RES_OK;   /* one-shot */
        return err;
    }

    if( s_recv_head >= s_recv_tail )
    {
        return RES_ERR_TIMEOUT;
    }

    *frame = s_recv_queue[ s_recv_head++ ];
    return RES_OK;
}
