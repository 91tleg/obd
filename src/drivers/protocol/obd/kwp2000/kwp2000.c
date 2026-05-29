/**
 * @file    kwp2000.c
 * @brief   ISO 14230 KWP2000 transport layer implementation
 */

#include "drivers/protocol/obd/kwp2000/kwp2000.h"
#include <stddef.h>
#include "drivers/protocol/obd/kwp2000/kwp2000_hw.h"
#include "drivers/protocol/obd/kwp2000/kwp2000_timing.h"
#include "lib/time/delay.h"

static uint8_t kwp_checksum( uint8_t const * data, uint32_t len )
{
    uint8_t sum = 0U;

    if( data != NULL )
    {
        uint32_t i = 0U;

        while( i < len )
        {
            sum += data[ i ];
            ++i;
        }
    }

    return sum;
}

static result_t send_byte_p4( uint8_t data )
{
    delay_timer_t timer;
    result_t result = RES_ERR_TIMEOUT;

    delay_timer_start( &timer, KWP_P4_MAX_MS );

    while( ( !delay_timer_expired( &timer ) ) &&
           ( RES_IS_FAILED( result ) ) )
    {
        result = kwp_hw_send_byte( data );
    }

    if( RES_IS_OK( result ) )
    {
        delay_ms( KWP_P4_MIN_MS );
    }

    return result;
}

static void send_5baud_byte( uint8_t byte )
{
    kwp_hw_bus_takeover();

    kwp_hw_line_low();
    delay_ms( KWP_5BAUD_BIT_MS );

    for( uint8_t i = 0U; i < 8U; ++i )
    {
        if( ( byte & ( 1U << i ) ) != 0U )
        {
            kwp_hw_line_high();
        }
        else
        {
            kwp_hw_line_low();
        }

        delay_ms( KWP_5BAUD_BIT_MS );
    }

    kwp_hw_line_high();
    delay_ms( KWP_5BAUD_BIT_MS );

    kwp_hw_bus_release();
}

result_t kwp_fast_init( void )
{
    result_t result;
    uint8_t sync = 0U;

    kwp_hw_bus_takeover();
    kwp_hw_line_low();
    delay_ms( KWP_W_LOW_MS );
    kwp_hw_line_high();
    delay_ms( KWP_W_HIGH_MS );
    kwp_hw_bus_release();

    kwp_hw_flush_rx();

    result = kwp_hw_recv_byte( &sync, KWP_SYNC_TIMEOUT_MS );

    if( RES_IS_OK( result ) )
    {
        if( sync != 0x55U )
        {
            result = RES_ERR_PROTOCOL;
        }
    }

    return result;
}

result_t kwp_5baud_init( uint8_t target_addr )
{
    result_t result;
    uint8_t sync = 0U;
    uint8_t kw1  = 0U;
    uint8_t kw2  = 0U;

    delay_ms( KWP_W5_MIN_MS );

    send_5baud_byte( target_addr );

    kwp_hw_flush_rx();

    result = kwp_hw_recv_byte( &sync, KWP_SYNC_TIMEOUT_MS );

    if( RES_IS_OK( result ) )
    {
        if( sync != 0x55U )
        {
            result = RES_ERR_PROTOCOL;
        }
    }

    if( RES_IS_OK( result ) )
    {
        result = kwp_hw_recv_byte( &kw1, KWP_KW_TIMEOUT_MS );
    }

    if( RES_IS_OK( result ) )
    {
        result = kwp_hw_recv_byte( &kw2, KWP_KW_TIMEOUT_MS );
    }

    if( RES_IS_OK( result ) )
    {
        uint8_t kw2_inv;
        delay_ms( KWP_W4_MIN_MS );
        kw2_inv = ( uint8_t )( ~kw2 );
        result  = kwp_hw_send_byte( kw2_inv );
    }

    if( RES_IS_OK( result ) )
    {
        uint8_t echo = 0U;
        ( void )kwp_hw_recv_byte( &echo, KWP_KW_TIMEOUT_MS );
    }

    return result;
}

result_t kwp_send_frame( kwp_frame_t const * frame )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( frame != NULL ) && ( frame->len > 0U ) )
    {
        uint8_t fmt = ( uint8_t )( KWP_FMT_BYTE | frame->len );
        uint8_t cs;
        uint8_t hdr[ 3U ];
        uint32_t i = 0U;

        hdr[ 0U ] = fmt;
        hdr[ 1U ] = frame->target;
        hdr[ 2U ] = frame->source;

        cs  = kwp_checksum( hdr, KWP_HDR_SIZE );
        cs += kwp_checksum( frame->data, ( uint32_t )frame->len );

        result = send_byte_p4( fmt );

        if( RES_IS_OK( result ) )
        {
            result = send_byte_p4( frame->target );
        }

        if( RES_IS_OK( result ) )
        {
            result = send_byte_p4( frame->source );
        }

        while( ( i < ( uint32_t )frame->len ) && ( RES_IS_OK( result ) ) )
        {
            result = send_byte_p4( frame->data[ i ] );
            ++i;
        }

        if( RES_IS_OK( result ) )
        {
            result = kwp_hw_send_byte( cs );
        }
    }

    return result;
}

result_t kwp_recv_frame( kwp_frame_t * frame, uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( frame != NULL )
    {
        uint8_t fmt   = 0U;
        uint8_t tgt   = 0U;
        uint8_t src   = 0U;
        uint8_t cs_rx = 0U;
        uint8_t len   = 0U;

        result = kwp_hw_recv_byte( &fmt, timeout_ms );

        if( RES_IS_OK( result ) )
        {
            result = kwp_hw_recv_byte( &tgt, KWP_P1_MAX_MS );
        }

        if( RES_IS_OK( result ) )
        {
            result = kwp_hw_recv_byte( &src, KWP_P1_MAX_MS );
        }

        if( RES_IS_OK( result ) )
        {
            len = fmt & 0x3FU;

            if( len == 0U )
            {
                result = RES_ERR_PROTOCOL;
            }
        }

        if( RES_IS_OK( result ) )
        {
            uint32_t i = 0U;

            while( ( i < ( uint32_t )len ) && ( RES_IS_OK( result ) ) )
            {
                result = kwp_hw_recv_byte( &frame->data[ i ], KWP_P1_MAX_MS );
                ++i;
            }
        }

        if( RES_IS_OK( result ) )
        {
            result = kwp_hw_recv_byte( &cs_rx, KWP_P1_MAX_MS );
        }

        if( RES_IS_OK( result ) )
        {
            uint8_t cs_calc;
            uint8_t hdr[ 3U ];

            hdr[ 0U ] = fmt;
            hdr[ 1U ] = tgt;
            hdr[ 2U ] = src;

            cs_calc  = kwp_checksum( hdr, KWP_HDR_SIZE );
            cs_calc += kwp_checksum( frame->data, ( uint32_t )len );

            if( cs_calc != cs_rx )
            {
                result = RES_ERR_CHECKSUM;
            }
        }

        if( RES_IS_OK( result ) )
        {
            frame->target = tgt;
            frame->source = src;
            frame->len    = ( uint8_t )len;
        }
    }

    return result;
}
