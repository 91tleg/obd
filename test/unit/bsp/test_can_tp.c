#include "unity.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "lib/core/result.h"

#include "hal/can.h"
#include "cmsis/stm32h753xx.h"   /* host */

static FDCAN_GlobalTypeDef s_dummy_periph;
#define BSP_CAN  ( &s_dummy_periph )

#define STUB_MAX_FRAMES  ( 128U )

static can_frame_t s_sent[ STUB_MAX_FRAMES ];
static uint32_t s_sent_count;

static can_frame_t s_recv_queue[ STUB_MAX_FRAMES ];
static uint32_t s_recv_head;
static uint32_t s_recv_tail;
static result_t s_recv_override;

void delay_ms( uint32_t ms )
{

}

result_t can_send( FDCAN_GlobalTypeDef * p_can, can_frame_t const * frame )
{
    ( void )p_can;
    if( s_sent_count < STUB_MAX_FRAMES )
    {
        s_sent[ s_sent_count++ ] = *frame;
    }
    return RES_OK;
}

result_t can_recv( FDCAN_GlobalTypeDef * p_can,
                   can_frame_t * frame,
                   uint32_t timeout_ms )
{
    ( void )p_can;
    ( void )timeout_ms;

    if( s_recv_override != RES_OK )
    {
        return s_recv_override;
    }

    if( s_recv_head >= s_recv_tail )
    {
        return RES_ERR_TIMEOUT;
    }

    *frame = s_recv_queue[ s_recv_head++ ];
    return RES_OK;
}

static void stub_reset( void )
{
    memset( s_sent, 0, sizeof( s_sent ) );
    memset( s_recv_queue, 0, sizeof( s_recv_queue ) );
    s_sent_count    = 0U;
    s_recv_head     = 0U;
    s_recv_tail     = 0U;
    s_recv_override = RES_OK;
}

static void stub_queue_recv( can_frame_t const * frame )
{
    if( s_recv_tail < STUB_MAX_FRAMES )
    {
        s_recv_queue[ s_recv_tail++ ] = *frame;
    }
}

static can_frame_t make_fc_cts( bool fd )
{
    can_frame_t fc;
    memset( &fc, 0, sizeof( fc ) );
    fc.id      = 0x7E8U;
    fc.fd      = fd;
    fc.dlc     = fd ? CAN_FD_DLC_64 : ( uint8_t )CAN_FRAME_DATA_LEN;
    fc.data[0] = 0x30U;   /* FC | CTS */
    fc.data[1] = 0U;      /* block size 0 */
    fc.data[2] = 0U;      /* STmin 0 */
    return fc;
}

static can_frame_t make_cf( uint8_t sn, uint8_t const * data,
                            uint32_t len, bool fd )
{
    can_frame_t cf;
    uint32_t max_data = fd ? 63U : 7U;
    uint32_t copy     = ( len < max_data ) ? len : max_data;

    memset( &cf, 0, sizeof( cf ) );
    cf.id      = 0x7E8U;
    cf.fd      = fd;
    cf.dlc     = fd ? CAN_FD_DLC_64 : ( uint8_t )CAN_FRAME_DATA_LEN;
    cf.data[0] = ( uint8_t )( 0x20U | ( sn & 0x0FU ) );
    memcpy( &cf.data[1], data, copy );
    return cf;
}

#include "drivers/protocol/obd/can/can_tp.h"

#ifndef CAN_TP_OBD_REQ_ID
#define CAN_TP_OBD_REQ_ID    ( 0x7DFU )
#endif
#ifndef CAN_TP_OBD_RESP_BASE
#define CAN_TP_OBD_RESP_BASE ( 0x7E8U )
#endif
#ifndef CAN_TP_OBD_RESP_MAX
#define CAN_TP_OBD_RESP_MAX  ( 0x7EFU )
#endif

static can_tp_ctx_t s_ctx;

void setUp( void )
{
    stub_reset();
    can_tp_init( &s_ctx );
}

void tearDown( void )
{
    /* Nothing to teardown */
}

void test_init_null_returns_invalid_arg( void )
{
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, can_tp_init( NULL ) );
}

void test_init_clears_context( void )
{
    s_ctx.rx_len      = 99U;
    s_ctx.sn_expected = 99U;
    s_ctx.use_fd      = true;

    TEST_ASSERT_EQUAL( RES_OK, can_tp_init( &s_ctx ) );
    TEST_ASSERT_EQUAL( 0U,     s_ctx.rx_len );
    TEST_ASSERT_EQUAL( 1U,     s_ctx.sn_expected );
    TEST_ASSERT_FALSE( s_ctx.use_fd );
}

void test_send_null_ctx_returns_invalid_arg( void )
{
    uint8_t buf[1] = { 0x01U };
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       can_tp_send( NULL, CAN_TP_OBD_REQ_ID, buf, 1U ) );
}

void test_send_null_payload_returns_invalid_arg( void )
{
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, NULL, 1U ) );
}

void test_send_zero_len_returns_invalid_arg( void )
{
    uint8_t buf[1] = { 0x01U };
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, buf, 0U ) );
}

void test_send_sf_classic_single_byte( void )
{
    uint8_t payload[1] = { 0xABU };
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 1U ) );

    TEST_ASSERT_EQUAL( 1U, s_sent_count );
    TEST_ASSERT_EQUAL( CAN_TP_OBD_REQ_ID, s_sent[0].id );
    /* PCI: [0000|0001] = 0x01 */
    TEST_ASSERT_EQUAL_HEX8( 0x01U, s_sent[0].data[0] );
    TEST_ASSERT_EQUAL_HEX8( 0xABU, s_sent[0].data[1] );
    TEST_ASSERT_FALSE( s_sent[0].fd );
}

void test_send_sf_classic_max_7_bytes( void )
{
    uint8_t payload[7] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U };
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 7U ) );

    TEST_ASSERT_EQUAL( 1U, s_sent_count );
    /* PCI: [0000|0111] = 0x07 */
    TEST_ASSERT_EQUAL_HEX8( 0x07U, s_sent[0].data[0] );
    TEST_ASSERT_EQUAL_MEMORY( payload, &s_sent[0].data[1], 7U );
}

void test_send_sf_classic_pads_unused_bytes( void )
{
    uint8_t payload[3] = { 0x01U, 0x02U, 0x03U };
    s_ctx.use_fd = false;

    can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 3U );

    /* bytes [4..7] must be 0x00 */
    for( uint32_t i = 4U; i < CAN_FRAME_DATA_LEN; ++i )
    {
        TEST_ASSERT_EQUAL_HEX8( 0x00U, s_sent[0].data[i] );
    }
}

void test_send_sf_fd_extended_pci( void )
{
    uint8_t payload[10];
    memset( payload, 0xBBU, sizeof( payload ) );
    s_ctx.use_fd = true;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 10U ) );

    TEST_ASSERT_EQUAL( 1U, s_sent_count );
    TEST_ASSERT_EQUAL_HEX8( 0x00U, s_sent[0].data[0] );   /* SF escape */
    TEST_ASSERT_EQUAL_HEX8( 10U,   s_sent[0].data[1] );   /* SF_DL     */
    TEST_ASSERT_EQUAL_MEMORY( payload, &s_sent[0].data[2], 10U );
    TEST_ASSERT_TRUE( s_sent[0].fd );
    TEST_ASSERT_EQUAL( CAN_FD_DLC_64, s_sent[0].dlc );
}

void test_send_sf_fd_max_62_bytes( void )
{
    uint8_t payload[62];
    for( uint32_t i = 0U; i < 62U; ++i )
    {
        payload[i] = ( uint8_t )i;
    }
    s_ctx.use_fd = true;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 62U ) );

    TEST_ASSERT_EQUAL( 1U, s_sent_count );
    TEST_ASSERT_EQUAL_HEX8( 0x00U, s_sent[0].data[0] );
    TEST_ASSERT_EQUAL_HEX8( 62U,   s_sent[0].data[1] );
    TEST_ASSERT_EQUAL_MEMORY( payload, &s_sent[0].data[2], 62U );
}

void test_send_ff_cf_classic_8_bytes( void )
{
    uint8_t payload[8];
    for( uint32_t i = 0U; i < 8U; ++i )
    {
        payload[i] = ( uint8_t )i;
    }
    s_ctx.use_fd = false;

    /* queue FC-CTS */
    can_frame_t fc;
    memset( &fc, 0, sizeof( fc ) );
    fc.id      = 0x7E8U;
    fc.data[0] = 0x30U;
    stub_queue_recv( &fc );

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 8U ) );

    TEST_ASSERT_EQUAL( 2U, s_sent_count );   /* FF + 1 CF */

    /* FF PCI: [0001|0000]=0x10, length=8 */
    TEST_ASSERT_EQUAL_HEX8( 0x10U, s_sent[0].data[0] );
    TEST_ASSERT_EQUAL_HEX8( 0x08U, s_sent[0].data[1] );
    TEST_ASSERT_EQUAL_MEMORY( payload, &s_sent[0].data[2], 6U );

    /* CF SN=1, remaining 2 bytes */
    TEST_ASSERT_EQUAL_HEX8( 0x21U,      s_sent[1].data[0] );
    TEST_ASSERT_EQUAL_HEX8( payload[6], s_sent[1].data[1] );
    TEST_ASSERT_EQUAL_HEX8( payload[7], s_sent[1].data[2] );
}

void test_send_sn_wraps_at_15( void )
{
    /*
     * 120 bytes: FF carries 6, each CF carries 7.
     * CFs needed: ceil((120-6)/7) = 17 -> SN wraps past 15.
     */
    uint8_t payload[120];
    for( uint32_t i = 0U; i < 120U; ++i )
    {
        payload[i] = ( uint8_t )i;
    }
    s_ctx.use_fd = false;

    can_frame_t fc;
    memset( &fc, 0, sizeof( fc ) );
    fc.id      = 0x7E8U;
    fc.data[0] = 0x30U;
    stub_queue_recv( &fc );

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 120U ) );

    /* frame 0 = FF, frames 1..N = CFs. SN of CF k = k & 0x0F */
    for( uint32_t k = 1U; k < s_sent_count; ++k )
    {
        uint8_t expected_sn = ( uint8_t )( k & 0x0FU );
        uint8_t actual_sn   = s_sent[k].data[0] & 0x0FU;
        TEST_ASSERT_EQUAL_HEX8( expected_sn, actual_sn );
    }
}

void test_send_ff_no_fc_returns_timeout( void )
{
    uint8_t payload[8] = { 0 };
    s_ctx.use_fd = false;
    /* no FC queued, can_recv returns RES_ERR_TIMEOUT */

    TEST_ASSERT_EQUAL( RES_ERR_TIMEOUT,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 8U ) );
}

void test_send_ff_wrong_fc_pci_returns_protocol( void )
{
    uint8_t payload[8] = { 0 };
    s_ctx.use_fd = false;

    can_frame_t bad_fc;
    memset( &bad_fc, 0, sizeof( bad_fc ) );
    bad_fc.id      = 0x7E8U;
    bad_fc.data[0] = 0x10U;   /* FF type */
    stub_queue_recv( &bad_fc );

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 8U ) );
}

void test_send_ff_fc_not_cts_returns_protocol( void )
{
    uint8_t payload[8] = { 0 };
    s_ctx.use_fd = false;

    can_frame_t wait_fc;
    memset( &wait_fc, 0, sizeof( wait_fc ) );
    wait_fc.id      = 0x7E8U;
    wait_fc.data[0] = 0x31U;   /* FC | WAIT */
    stub_queue_recv( &wait_fc );

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 8U ) );
}

void test_send_ff_fd_std_header( void )
{
    /* FD FF (62 bytes data) + 1 CF (1 byte) */
    uint8_t  payload[63];
    for( uint32_t i = 0U; i < 63U; ++i )
    {
        payload[i] = ( uint8_t )i;
    }
    s_ctx.use_fd = true;

    can_frame_t fc = make_fc_cts( true );
    stub_queue_recv( &fc );

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 63U ) );

    /* FF: data[0]=0x10|(63>>8)=0x10, data[1]=63=0x3F */
    TEST_ASSERT_EQUAL_HEX8( 0x10U, s_sent[0].data[0] );
    TEST_ASSERT_EQUAL_HEX8( 0x3FU, s_sent[0].data[1] );
    TEST_ASSERT_EQUAL_MEMORY( payload, &s_sent[0].data[2], 62U );
    TEST_ASSERT_TRUE( s_sent[0].fd );
}

void test_send_ff_fd_escape_header( void )
{
    static uint8_t payload[4096];
    for( uint32_t i = 0U; i < 4096U; ++i )
    {
        payload[i] = ( uint8_t )( i & 0xFFU );
    }
    s_ctx.use_fd = true;

    can_frame_t fc = make_fc_cts( true );
    stub_queue_recv( &fc );

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_send( &s_ctx, CAN_TP_OBD_REQ_ID, payload, 4096U ) );

    /* Escape FF: data[0]=0x10, data[1]=0x00, data[2..5]=length big-endian */
    TEST_ASSERT_EQUAL_HEX8( 0x10U, s_sent[0].data[0] );
    TEST_ASSERT_EQUAL_HEX8( 0x00U, s_sent[0].data[1] );
    TEST_ASSERT_EQUAL_HEX8( 0x00U, s_sent[0].data[2] );   /* 4096 >> 24 */
    TEST_ASSERT_EQUAL_HEX8( 0x00U, s_sent[0].data[3] );   /* 4096 >> 16 */
    TEST_ASSERT_EQUAL_HEX8( 0x10U, s_sent[0].data[4] );   /* 4096 >>  8 */
    TEST_ASSERT_EQUAL_HEX8( 0x00U, s_sent[0].data[5] );   /* 4096 & 0xFF */
    TEST_ASSERT_EQUAL_MEMORY( payload, &s_sent[0].data[6], 58U );
    TEST_ASSERT_TRUE( s_sent[0].fd );
}

void test_recv_null_ctx_returns_invalid_arg( void )
{
    uint8_t  buf[8];
    uint32_t len;
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       can_tp_recv( NULL, buf, sizeof( buf ), &len, 100U ) );
}

void test_recv_null_buf_returns_invalid_arg( void )
{
    uint32_t len;
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       can_tp_recv( &s_ctx, NULL, 8U, &len, 100U ) );
}

void test_recv_null_out_len_returns_invalid_arg( void )
{
    uint8_t buf[8];
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), NULL, 100U ) );
}

void test_recv_zero_buf_size_returns_invalid_arg( void )
{
    uint8_t  buf[8];
    uint32_t len;
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       can_tp_recv( &s_ctx, buf, 0U, &len, 100U ) );
}

void test_recv_out_len_zero_on_timeout( void )
{
    uint8_t  buf[8];
    uint32_t len = 0xDEADBEEFU;

    can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 10U );
    TEST_ASSERT_EQUAL( 0U, len );
}

void test_recv_out_len_zero_on_wrong_id( void )
{
    uint8_t  buf[8];
    uint32_t len = 0xDEADBEEFU;

    can_frame_t frame;
    memset( &frame, 0, sizeof( frame ) );
    frame.id      = 0x123U;
    frame.data[0] = 0x01U;
    frame.data[1] = 0xAAU;
    stub_queue_recv( &frame );

    can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 10U );
    TEST_ASSERT_EQUAL( 0U, len );
}

void test_recv_sf_classic_single_byte( void )
{
    can_frame_t frame;
    memset( &frame, 0, sizeof( frame ) );
    frame.id      = 0x7E8U;
    frame.data[0] = 0x01U;
    frame.data[1] = 0xAAU;
    stub_queue_recv( &frame );

    uint8_t  buf[8];
    uint32_t len = 0U;
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
    TEST_ASSERT_EQUAL( 1U,    len );
    TEST_ASSERT_EQUAL_HEX8( 0xAAU, buf[0] );
}

void test_recv_sf_classic_7_bytes( void )
{
    can_frame_t frame;
    uint32_t    i;
    memset( &frame, 0, sizeof( frame ) );
    frame.id      = 0x7E8U;
    frame.data[0] = 0x07U;
    for( i = 1U; i <= 7U; ++i )
    {
        frame.data[i] = ( uint8_t )i;
    }
    stub_queue_recv( &frame );

    uint8_t  buf[8];
    uint32_t len = 0U;
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
    TEST_ASSERT_EQUAL( 7U, len );
    for( i = 0U; i < 7U; ++i )
    {
        TEST_ASSERT_EQUAL_HEX8( ( uint8_t )( i + 1U ), buf[i] );
    }
}

void test_recv_sf_classic_truncates_to_buf_size( void )
{
    can_frame_t frame;
    memset( &frame, 0, sizeof( frame ) );
    frame.id      = 0x7E8U;
    frame.data[0] = 0x07U;
    for( uint32_t i = 1U; i <= 7U; ++i )
    {
        frame.data[i] = ( uint8_t )i;
    }
    stub_queue_recv( &frame );

    uint8_t  buf[3];
    uint32_t len = 0U;

    can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U );
    TEST_ASSERT_EQUAL( 3U, len );
}

void test_recv_sf_fd_extended_pci( void )
{
    can_frame_t frame;
    uint32_t    i;
    memset( &frame, 0, sizeof( frame ) );
    frame.id      = 0x7E8U;
    frame.fd      = true;
    frame.dlc     = CAN_FD_DLC_64;
    frame.data[0] = 0x00U;
    frame.data[1] = 10U;
    for( i = 0U; i < 10U; ++i )
    {
        frame.data[2U + i] = ( uint8_t )( i + 1U );
    }
    stub_queue_recv( &frame );

    uint8_t  buf[64];
    uint32_t len = 0U;
    s_ctx.use_fd = true;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
    TEST_ASSERT_EQUAL( 10U, len );
    for( i = 0U; i < 10U; ++i )
    {
        TEST_ASSERT_EQUAL_HEX8( ( uint8_t )( i + 1U ), buf[i] );
    }
}

void test_recv_ff_cf_classic_8_bytes( void )
{
    uint8_t  expected[8];
    for( uint32_t i = 0U; i < 8U; ++i )
    {
        expected[i] = ( uint8_t )( i + 1U );
    }

    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.data[0] = 0x10U;
    ff.data[1] = 0x08U;
    memcpy( &ff.data[2], expected, 6U );
    stub_queue_recv( &ff );

    can_frame_t cf = make_cf( 1U, &expected[6], 2U, false );
    stub_queue_recv( &cf );

    uint8_t  buf[8];
    uint32_t len = 0U;
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
    TEST_ASSERT_EQUAL( 8U, len );
    TEST_ASSERT_EQUAL_MEMORY( expected, buf, 8U );

    /* FC-CTS must have been sent */
    TEST_ASSERT_EQUAL( 1U, s_sent_count );
    TEST_ASSERT_EQUAL_HEX8( 0x30U, s_sent[0].data[0] );
}

void test_recv_ff_cf_classic_sn_sequence( void )
{
    /* 20 bytes: FF=6, CF1=7, CF2=7 */
    uint8_t  expected[20];
    for( uint32_t i = 0U; i < 20U; ++i )
    {
        expected[i] = ( uint8_t )( i + 1U );
    }

    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.data[0] = 0x10U;
    ff.data[1] = 20U;
    memcpy( &ff.data[2], expected, 6U );
    stub_queue_recv( &ff );

    can_frame_t cf1 = make_cf( 1U, &expected[6],  7U, false );
    can_frame_t cf2 = make_cf( 2U, &expected[13], 7U, false );
    stub_queue_recv( &cf1 );
    stub_queue_recv( &cf2 );

    uint8_t  buf[20];
    uint32_t len = 0U;
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
    TEST_ASSERT_EQUAL( 20U, len );
    TEST_ASSERT_EQUAL_MEMORY( expected, buf, 20U );
}

void test_recv_wrong_id_returns_protocol_err( void )
{
    can_frame_t frame;
    memset( &frame, 0, sizeof( frame ) );
    frame.id      = 0x123U;
    frame.data[0] = 0x01U;
    stub_queue_recv( &frame );

    uint8_t  buf[8];
    uint32_t len;
    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
}

void test_recv_unexpected_pci_type_returns_protocol( void )
{
    can_frame_t frame;
    memset( &frame, 0, sizeof( frame ) );
    frame.id      = 0x7E8U;
    frame.data[0] = 0x30U;   /* FC */
    stub_queue_recv( &frame );

    uint8_t  buf[8];
    uint32_t len;
    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
}

void test_recv_wrong_cf_sn_returns_protocol( void )
{
    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.data[0] = 0x10U;
    ff.data[1] = 0x08U;
    stub_queue_recv( &ff );

    /* Wrong SN: should be 1, sending 2 */
    uint8_t     bad_data[2] = { 1U, 2U };
    can_frame_t bad_cf = make_cf( 2U, bad_data, 2U, false );
    stub_queue_recv( &bad_cf );

    uint8_t  buf[8];
    uint32_t len;
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
}

void test_recv_wrong_cf_pci_returns_protocol( void )
{
    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.data[0] = 0x10U;
    ff.data[1] = 0x08U;
    stub_queue_recv( &ff );

    /* SF instead of CF */
    can_frame_t bad;
    memset( &bad, 0, sizeof( bad ) );
    bad.id      = 0x7E8U;
    bad.data[0] = 0x01U;
    stub_queue_recv( &bad );

    uint8_t  buf[8];
    uint32_t len;
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
}

void test_recv_stray_cf_id_returns_protocol( void )
{
    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.data[0] = 0x10U;
    ff.data[1] = 0x08U;
    stub_queue_recv( &ff );

    can_frame_t stray;
    memset( &stray, 0, sizeof( stray ) );
    stray.id      = 0x123U;   /* wrong ID */
    stray.data[0] = 0x21U;    /* CF SN=1 */
    stub_queue_recv( &stray );

    uint8_t  buf[8];
    uint32_t len;
    s_ctx.use_fd = false;

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
}

void test_recv_sn_reset_after_error( void )
{
    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.data[0] = 0x10U;
    ff.data[1] = 0x08U;
    stub_queue_recv( &ff );

    uint8_t     bad_data[2] = { 1U, 2U };
    can_frame_t bad_cf = make_cf( 5U, bad_data, 2U, false );  /* wrong SN */
    stub_queue_recv( &bad_cf );

    uint8_t  buf[8];
    uint32_t len;
    s_ctx.use_fd = false;
    can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U );

    /* sn_expected must be reset to 1 after error */
    TEST_ASSERT_EQUAL( 1U, s_ctx.sn_expected );
}

void test_recv_ff_cf_fd_std( void )
{
    /* 63 bytes total: FF carries 62, CF carries 1 */
    uint8_t  expected[63];
    for( uint32_t i = 0U; i < 63U; ++i )
    {
        expected[i] = ( uint8_t )( i + 1U );
    }

    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.fd      = true;
    ff.dlc     = CAN_FD_DLC_64;
    ff.data[0] = 0x10U;
    ff.data[1] = 63U;
    memcpy( &ff.data[2], expected, 62U );
    stub_queue_recv( &ff );

    can_frame_t cf = make_cf( 1U, &expected[62], 1U, true );
    stub_queue_recv( &cf );

    uint8_t  buf[64];
    uint32_t len = 0U;
    s_ctx.use_fd = true;

    TEST_ASSERT_EQUAL( RES_OK,
                       can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U ) );
    TEST_ASSERT_EQUAL( 63U, len );
    TEST_ASSERT_EQUAL_MEMORY( expected, buf, 63U );
}

void test_recv_ff_fd_escape( void )
{
    uint32_t       total = 4096U;
    static uint8_t expected[4096];
    for( uint32_t i = 0U; i < 4096U; ++i )
    {
        expected[i] = ( uint8_t )( i & 0xFFU );
    }

    /* Build escape FF */
    can_frame_t ff;
    memset( &ff, 0, sizeof( ff ) );
    ff.id      = 0x7E8U;
    ff.fd      = true;
    ff.dlc     = CAN_FD_DLC_64;
    ff.data[0] = 0x10U;
    ff.data[1] = 0x00U;
    ff.data[2] = ( uint8_t )( total >> 24U );
    ff.data[3] = ( uint8_t )( total >> 16U );
    ff.data[4] = ( uint8_t )( total >>  8U );
    ff.data[5] = ( uint8_t )( total        );
    memcpy( &ff.data[6], expected, 58U );
    stub_queue_recv( &ff );

    /* Queue remaining CFs */
    uint32_t offset = 58U;
    uint8_t  sn     = 1U;
    while( offset < total )
    {
        uint32_t chunk = ( ( total - offset ) < 63U )
                         ? ( total - offset ) : 63U;
        can_frame_t cf = make_cf( sn, &expected[offset], chunk, true );
        stub_queue_recv( &cf );
        offset += chunk;
        sn = ( uint8_t )( ( sn + 1U ) & 0x0FU );
    }

    uint32_t queued_before = s_recv_tail;

    static uint8_t buf[4096];
    uint32_t len = 0U;
    s_ctx.use_fd = true;

    result_t r = can_tp_recv( &s_ctx, buf, sizeof( buf ), &len, 100U );

    uint32_t remaining = s_recv_tail - s_recv_head;

    TEST_ASSERT_EQUAL( 66U, queued_before );     /* 1 FF + 65 CFs */
    TEST_ASSERT_EQUAL( 0U,  remaining );         /* all consumed  */
    TEST_ASSERT_EQUAL( RES_OK, r );
    TEST_ASSERT_EQUAL( 4096U, len );
    TEST_ASSERT_EQUAL_MEMORY( expected, buf, 4096U );
}

int main( void )
{
    UNITY_BEGIN();

    RUN_TEST( test_init_null_returns_invalid_arg );
    RUN_TEST( test_init_clears_context );

    RUN_TEST( test_send_null_ctx_returns_invalid_arg );
    RUN_TEST( test_send_null_payload_returns_invalid_arg );
    RUN_TEST( test_send_zero_len_returns_invalid_arg );

    RUN_TEST( test_send_sf_classic_single_byte );
    RUN_TEST( test_send_sf_classic_max_7_bytes );
    RUN_TEST( test_send_sf_classic_pads_unused_bytes );

    RUN_TEST( test_send_sf_fd_extended_pci );
    RUN_TEST( test_send_sf_fd_max_62_bytes );

    RUN_TEST( test_send_ff_cf_classic_8_bytes );
    RUN_TEST( test_send_sn_wraps_at_15 );
    RUN_TEST( test_send_ff_no_fc_returns_timeout );
    RUN_TEST( test_send_ff_wrong_fc_pci_returns_protocol );
    RUN_TEST( test_send_ff_fc_not_cts_returns_protocol );

    RUN_TEST( test_send_ff_fd_std_header );
    RUN_TEST( test_send_ff_fd_escape_header );

    RUN_TEST( test_recv_null_ctx_returns_invalid_arg );
    RUN_TEST( test_recv_null_buf_returns_invalid_arg );
    RUN_TEST( test_recv_null_out_len_returns_invalid_arg );
    RUN_TEST( test_recv_zero_buf_size_returns_invalid_arg );

    RUN_TEST( test_recv_out_len_zero_on_timeout );
    RUN_TEST( test_recv_out_len_zero_on_wrong_id );

    RUN_TEST( test_recv_sf_classic_single_byte );
    RUN_TEST( test_recv_sf_classic_7_bytes );
    RUN_TEST( test_recv_sf_classic_truncates_to_buf_size );

    RUN_TEST( test_recv_sf_fd_extended_pci );

    RUN_TEST( test_recv_ff_cf_classic_8_bytes );
    RUN_TEST( test_recv_ff_cf_classic_sn_sequence );

    RUN_TEST( test_recv_wrong_id_returns_protocol_err );
    RUN_TEST( test_recv_unexpected_pci_type_returns_protocol );
    RUN_TEST( test_recv_wrong_cf_sn_returns_protocol );
    RUN_TEST( test_recv_wrong_cf_pci_returns_protocol );
    RUN_TEST( test_recv_stray_cf_id_returns_protocol );
    RUN_TEST( test_recv_sn_reset_after_error );

    RUN_TEST( test_recv_ff_cf_fd_std );
    RUN_TEST( test_recv_ff_fd_escape );

    return UNITY_END();
}
