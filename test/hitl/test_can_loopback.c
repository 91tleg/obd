/**
 * @file test_can_loopback.c
 * @brief HITL loopback test suite.
 */

#include <stdint.h>

#include "hal/can.h"
#include "lib/core/result.h"
#include "lib/log/log.h"
#include "lib/string/str.h"
#include "lib/time/delay_tick.h"
#include "drivers/protocol/obd/can/can_tp.h"

#define TAG  "HITL"

#define TEST_FILTER_LOW   ( 0x7E8U )
#define TEST_FILTER_HIGH  ( 0x7EFU )

#define TEST_NBTP  CAN_NBTP( 10U, 1U, 29U, 10U )

static uint32_t s_pass = 0U;
static uint32_t s_fail = 0U;

#define CHECK( cond, name )                           \
    do {                                              \
        if( ( cond ) ) {                              \
            LOGI( TAG, name ": PASS" );               \
            s_pass++;                                 \
        } else {                                      \
            LOGE( TAG, name ": FAIL" );               \
            s_fail++;                                 \
        }                                             \
    } while( 0 )

static result_t init_loopback( void )
{
    can_filter_t filter = { .id_low = TEST_FILTER_LOW, .id_high = TEST_FILTER_HIGH };
    return can_init_loopback( FDCAN1, TEST_NBTP, 0U, &filter );
}

static void drain_rx_fifo( void )
{
    can_frame_t f;
    while( RES_IS_OK( can_recv( FDCAN1, &f, 1U ) ) )
    {
        /* discard */
    }
}

static void test_1_classic_sf( void )
{
    can_frame_t tx = { .id = 0x7E8U, .dlc = 8U, .fd = false, .brs = false };
    can_frame_t rx;

    mem_set( tx.data, 0xAAU, CAN_FRAME_DATA_LEN );

    result_t r = can_send( FDCAN1, &tx );
    CHECK( RES_IS_OK( r ), "T1 send" );

    r = can_recv( FDCAN1, &rx, 10U );
    CHECK( RES_IS_OK( r ), "T1 recv" );
    CHECK( rx.id == tx.id, "T1 id" );
    CHECK( rx.dlc == 8U, "T1 dlc" );
    CHECK( rx.fd == false, "T1 fd flag" );
    CHECK( mem_equal( tx.data, rx.data, CAN_FRAME_DATA_LEN ) == 0, "T1 data" );
}

static void test_2_fd_64( void )
{
    can_frame_t tx = { .id = 0x7E8U, .dlc = CAN_FD_DLC_64, .fd = true, .brs = false };
    can_frame_t rx;

    for( uint32_t i = 0U; i < CAN_FRAME_FD_DATA_LEN; ++i )
    {
        tx.data[ i ] = ( uint8_t )i;
    }

    result_t r = can_send( FDCAN1, &tx );
    CHECK( RES_IS_OK( r ), "T2 send" );

    r = can_recv( FDCAN1, &rx, 10U );
    CHECK( RES_IS_OK( r ), "T2 recv" );
    CHECK( rx.id == tx.id, "T2 id" );
    CHECK( rx.dlc == CAN_FD_DLC_64, "T2 dlc" );
    CHECK( rx.fd == true, "T2 fd flag" );
    CHECK( mem_equal( tx.data, rx.data, CAN_FRAME_FD_DATA_LEN ) == 0, "T2 data" );
}

static void test_3_dlc_sweep( void )
{
    static const uint8_t  dlc_values[ 16U ] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    static const uint32_t expected_bytes[ 16U ] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };

    for( uint32_t i = 0U; i < 16U; ++i )
    {
        can_frame_t tx = { .id = 0x7E8U, .dlc = dlc_values[ i ], .fd = true, .brs = false };
        can_frame_t rx;

        for( uint32_t b = 0U; b < expected_bytes[ i ]; ++b )
        {
            tx.data[ b ] = ( uint8_t )( b + i );
        }

        result_t r = can_send( FDCAN1, &tx );
        if( !RES_IS_OK( r ) ) { LOGE_U32( TAG, "T3 send fail dlc=", dlc_values[ i ] ); s_fail++; continue; }

        r = can_recv( FDCAN1, &rx, 10U );
        if( !RES_IS_OK( r ) ) { LOGE_U32( TAG, "T3 recv fail dlc=", dlc_values[ i ] ); s_fail++; continue; }

        bool ok = ( rx.dlc == dlc_values[ i ] ) &&
                  ( mem_equal( tx.data, rx.data, expected_bytes[ i ] ) == 0 );
        CHECK( ok, "T3 dlc roundtrip" );
    }
}

static void test_4_id_filter( void )
{
    struct { uint32_t id; bool should_pass; } cases[] =
    {
        { 0x7E7U, false },
        { 0x7E8U, true  },
        { 0x7EFU, true  },
        { 0x7F0U, false },
    };

    for( uint32_t i = 0U; i < 4U; ++i )
    {
        can_frame_t tx = { .id = cases[ i ].id, .dlc = 8U, .fd = false, .brs = false };
        can_frame_t rx;
        mem_set( tx.data, 0x55U, CAN_FRAME_DATA_LEN );

        result_t sr = can_send( FDCAN1, &tx );
        if( !RES_IS_OK( sr ) ) { LOGE_U32( TAG, "T4 send fail id=", cases[ i ].id ); s_fail++; continue; }

        result_t rr = can_recv( FDCAN1, &rx, 20U );

        if( cases[ i ].should_pass )
        {
            CHECK( ( rr == RES_OK ) && ( rx.id == cases[ i ].id ), "T4 accept in range" );
        }
        else
        {
            CHECK( rr == RES_ERR_TIMEOUT, "T4 reject out of range" );
        }
    }
}

static void test_5_tx_fifo_full( void )
{
    can_frame_t tx = { .id = 0x7E8U, .dlc = 8U, .fd = false, .brs = false };
    mem_set( tx.data, 0x11U, CAN_FRAME_DATA_LEN );

    uint32_t sent = 0U;
    result_t r = RES_OK;

    for( uint32_t i = 0U; i < 8U; ++i )
    {
        r = can_send( FDCAN1, &tx );
        if( RES_IS_OK( r ) ) { sent++; }
    }
    CHECK( sent == 8U, "T5 fill 8 slots" );

    r = can_send( FDCAN1, &tx );
    CHECK( r == RES_ERR_BUSY, "T5 9th send RES_ERR_BUSY" );

    /* drain everything before moving on. */
    drain_rx_fifo();
}

static void test_6_rx_overflow( void )
{
    can_frame_t tx = { .id = 0x7E8U, .dlc = 8U, .fd = false, .brs = false };
    mem_set( tx.data, 0x22U, CAN_FRAME_DATA_LEN );

    /* RX FIFO0 holds 8 elements; send 9 without draining. */
    for( uint32_t i = 0U; i < 9U; ++i )
    {
        result_t r = can_send( FDCAN1, &tx );
        if( !RES_IS_OK( r ) )
        {
            LOGW_U32( TAG, "T6 send stalled at frame ", i );
        }
        /* small spin so hardware has time to complete each TX before next */
        for( volatile uint32_t d = 0U; d < 2000U; ++d ) { }
    }

    bool overflow_set = ( FDCAN1->RXF0S & FDCAN_RXF0S_RF0L ) != 0U;
    CHECK( overflow_set, "T6 RX FIFO overflow flag set" );

    drain_rx_fifo();
}

static void test_7_cantp_sf_classic( void )
{
    can_tp_ctx_t ctx;
    can_tp_init( &ctx );
    ctx.use_fd = false;

    uint8_t payload[ 5U ] = { 0x01U, 0x02U, 0x03U, 0x04U, 0x05U };
    uint8_t rxbuf[ 32U ];
    uint32_t out_len = 0U;

    result_t sr = can_tp_send( &ctx, 0x7E8U, payload, sizeof( payload ) );
    CHECK( RES_IS_OK( sr ), "T7 can_tp_send" );

    result_t rr = can_tp_recv( &ctx, rxbuf, sizeof( rxbuf ), &out_len, 20U );
    CHECK( RES_IS_OK( rr ), "T7 can_tp_recv" );
    CHECK( out_len == sizeof( payload ), "T7 out_len" );
    CHECK( mem_equal( payload, rxbuf, sizeof( payload ) ) == 0, "T7 payload match" );
}

static void test_9_cantp_sf_fd( void )
{
    can_tp_ctx_t ctx;
    can_tp_init( &ctx );
    ctx.use_fd = true;

    uint8_t payload[ 50U ];
    for( uint32_t i = 0U; i < sizeof( payload ); ++i ) { payload[ i ] = ( uint8_t )( i * 3U ); }

    uint8_t rxbuf[ 64U ];
    uint32_t out_len = 0U;

    result_t sr = can_tp_send( &ctx, 0x7E8U, payload, sizeof( payload ) );
    CHECK( RES_IS_OK( sr ), "T9 can_tp_send" );

    result_t rr = can_tp_recv( &ctx, rxbuf, sizeof( rxbuf ), &out_len, 20U );
    CHECK( RES_IS_OK( rr ), "T9 can_tp_recv" );
    CHECK( out_len == sizeof( payload ), "T9 out_len" );
    CHECK( mem_equal( payload, rxbuf, sizeof( payload ) ) == 0, "T9 payload match" );
}

void run_can_loopback_tests( void )
{
    LOGI( TAG, "---------- CAN loopback HITL starting ----------" );

    result_t init_res = init_loopback();
    if( !RES_IS_OK( init_res ) )
    {
        LOGE( TAG, "can_init_loopback failed." );
        return;
    }

    test_1_classic_sf();
    drain_rx_fifo();

    test_2_fd_64();
    drain_rx_fifo();

    test_3_dlc_sweep();
    drain_rx_fifo();

    test_4_id_filter();
    drain_rx_fifo();

    test_5_tx_fifo_full();
    test_6_rx_overflow();

    test_7_cantp_sf_classic();
    drain_rx_fifo();

    test_9_cantp_sf_fd();
    drain_rx_fifo();

    LOGI_U32( TAG, "PASS: ", s_pass );
    LOGI_U32( TAG, "FAIL: ", s_fail );
    LOGI( TAG, "---------- CAN loopback HITL done ----------" );
}
