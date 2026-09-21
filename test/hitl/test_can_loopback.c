/**
 * @file test_can_loopback.c
 * @brief HITL loopback test suite (FDCAN1 internal loopback, no bus needed).
 *
 * Results are kept in RAM (hitl_pass / hitl_fail / hitl_fails[] / hitl_done)
 * so scripts/hitl.sh can read them over SWD without relying on the UART.
 * hitl_finish() is the anchor the script breaks on.
 */

#include <stddef.h>
#include <stdint.h>

#include "bsp/nucleo_h753zi/wdt.h"
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
/* Data phase at twice the nominal rate: 20 tq per bit vs 40. */
#define TEST_DBTP  CAN_DBTP( 4U, 1U, 15U, 4U )

#define TX_IDLE_TIMEOUT_MS  ( 20U )

#define HITL_MAX_FAILS  ( 32U )

typedef struct
{
    char const * name;
    uint32_t     line;
} hitl_fail_t;

/* Non-static on purpose: read by scripts/hitl.sh through GDB/OpenOCD. */
volatile uint32_t hitl_pass;
volatile uint32_t hitl_fail;
volatile uint32_t hitl_done;
hitl_fail_t       hitl_fails[ HITL_MAX_FAILS ];

static void record_fail( char const * name, uint32_t line )
{
    if( hitl_fail < HITL_MAX_FAILS )
    {
        hitl_fails[ hitl_fail ].name = name;
        hitl_fails[ hitl_fail ].line = line;
    }
    hitl_fail++;
    LOGE( TAG, name );
}

#define CHECK( cond, name )                           \
    do {                                              \
        bsp_wdt_refresh();                            \
        if( ( cond ) ) {                              \
            LOGI( TAG, name ": PASS" );               \
            hitl_pass++;                              \
        } else {                                      \
            record_fail( name ": FAIL", __LINE__ );   \
        }                                             \
    } while( 0 )

/* ------------------------------------------------------------------ */
/* helpers                                                            */
/* ------------------------------------------------------------------ */

static result_t init_loopback_with( can_data_timing_t data_timing )
{
    can_filter_t filter = { .id_low = TEST_FILTER_LOW, .id_high = TEST_FILTER_HIGH };
    return can_init_loopback( FDCAN1, TEST_NBTP, data_timing, &filter );
}

static void drain_rx_fifo( void )
{
    can_frame_t f;
    while( RES_IS_OK( can_recv( FDCAN1, &f, 1U ) ) )
    {
        /* discard */
    }
}

/** Wait until every queued TX element has left the FIFO. */
static bool wait_tx_idle( void )
{
    uint32_t const start = delay_get_tick();

    while( ( FDCAN1->TXBRP != 0U ) &&
           ( ( delay_get_tick() - start ) < TX_IDLE_TIMEOUT_MS ) )
    {
        /* spin */
    }

    return ( FDCAN1->TXBRP == 0U );
}

static can_frame_t make_classic( uint32_t id, uint8_t fill )
{
    can_frame_t f = { .id = id, .dlc = 8U, .fd = false, .brs = false };
    mem_set( f.data, fill, CAN_FRAME_DATA_LEN );
    return f;
}

/* ------------------------------------------------------------------ */
/* raw FDCAN HAL                                                      */
/* ------------------------------------------------------------------ */

static void test_1_classic_sf( void )
{
    can_frame_t tx = make_classic( 0x7E8U, 0xAAU );
    can_frame_t rx;

    result_t r = can_send( FDCAN1, &tx );
    CHECK( RES_IS_OK( r ), "T1 send" );

    r = can_recv( FDCAN1, &rx, 10U );
    CHECK( RES_IS_OK( r ), "T1 recv" );
    CHECK( rx.id == tx.id, "T1 id" );
    CHECK( rx.dlc == 8U, "T1 dlc" );
    CHECK( rx.fd == false, "T1 fd flag" );
    CHECK( mem_equal( tx.data, rx.data, CAN_FRAME_DATA_LEN ), "T1 data" );
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
    CHECK( mem_equal( tx.data, rx.data, CAN_FRAME_FD_DATA_LEN ), "T2 data" );
}

static void test_3_dlc_sweep( void )
{
    static const uint32_t expected_bytes[ 16U ] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };

    for( uint32_t dlc = 0U; dlc < 16U; ++dlc )
    {
        can_frame_t tx = { .id = 0x7E8U, .dlc = ( uint8_t )dlc, .fd = true, .brs = false };
        can_frame_t rx;

        for( uint32_t b = 0U; b < expected_bytes[ dlc ]; ++b )
        {
            tx.data[ b ] = ( uint8_t )( b + dlc );
        }

        result_t const sr = can_send( FDCAN1, &tx );
        result_t const rr = RES_IS_OK( sr ) ? can_recv( FDCAN1, &rx, 10U )
                                            : RES_ERR_INTERNAL;

        bool const ok = RES_IS_OK( sr ) && RES_IS_OK( rr ) &&
                        ( rx.dlc == dlc ) &&
                        ( mem_equal( tx.data, rx.data, expected_bytes[ dlc ] ) );

        if( !ok )
        {
            LOGE_U32( TAG, "T3 dlc failed: ", dlc );
        }
        CHECK( ok, "T3 dlc roundtrip" );
    }
}

static void test_4_id_filter( void )
{
    struct { uint32_t id; bool should_pass; } const cases[] =
    {
        { 0x000U, false },
        { 0x7E7U, false },
        { 0x7E8U, true  },
        { 0x7EBU, true  },
        { 0x7EFU, true  },
        { 0x7F0U, false },
        { 0x7FFU, false },
    };

    for( uint32_t i = 0U; i < ( sizeof( cases ) / sizeof( cases[ 0 ] ) ); ++i )
    {
        can_frame_t tx = make_classic( cases[ i ].id, 0x55U );
        can_frame_t rx;

        result_t const sr = can_send( FDCAN1, &tx );
        CHECK( RES_IS_OK( sr ), "T4 send" );

        result_t const rr = can_recv( FDCAN1, &rx, 20U );

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
    can_frame_t tx = make_classic( 0x7E8U, 0x11U );
    uint32_t sent = 0U;

    for( uint32_t i = 0U; i < 8U; ++i )
    {
        if( RES_IS_OK( can_send( FDCAN1, &tx ) ) )
        {
            sent++;
        }
    }
    CHECK( sent == 8U, "T5 fill 8 slots" );

    result_t const r = can_send( FDCAN1, &tx );
    CHECK( r == RES_ERR_BUSY, "T5 9th send RES_ERR_BUSY" );

    CHECK( wait_tx_idle(), "T5 TX FIFO drains" );
    drain_rx_fifo();
}

static void test_6_rx_overflow( void )
{
    can_frame_t tx = make_classic( 0x7E8U, 0x22U );
    bool all_sent = true;

    /* RX FIFO0 holds 8 elements; send 9 without draining. */
    for( uint32_t i = 0U; i < 9U; ++i )
    {
        all_sent = all_sent && RES_IS_OK( can_send( FDCAN1, &tx ) );
        CHECK( wait_tx_idle(), "T6 TX completes" );
    }
    CHECK( all_sent, "T6 9 sends accepted" );

    CHECK( ( FDCAN1->RXF0S & FDCAN_RXF0S_RF0L ) != 0U, "T6 RX FIFO overflow flag set" );
    CHECK( ( ( FDCAN1->RXF0S & FDCAN_RXF0S_F0FL_Msk ) >> FDCAN_RXF0S_F0FL_Pos ) == 8U,
           "T6 RX FIFO holds 8" );

    drain_rx_fifo();
}

static void test_7_ordering( void )
{
    can_frame_t rx;
    bool in_order = true;

    for( uint8_t i = 0U; i < 8U; ++i )
    {
        can_frame_t tx = make_classic( 0x7E8U, i );
        in_order = in_order && RES_IS_OK( can_send( FDCAN1, &tx ) );
    }
    CHECK( in_order, "T7 8 sends accepted" );
    CHECK( wait_tx_idle(), "T7 TX completes" );

    for( uint8_t i = 0U; i < 8U; ++i )
    {
        bool const got = RES_IS_OK( can_recv( FDCAN1, &rx, 10U ) ) &&
                         ( rx.data[ 0 ] == i );
        in_order = in_order && got;
    }
    CHECK( in_order, "T7 frames received in send order" );
}

static void test_8_recv_edge_cases( void )
{
    can_frame_t rx;
    can_frame_t tx = make_classic( 0x7E8U, 0x00U );

    CHECK( can_recv( NULL, &rx, 1U ) == RES_ERR_INVALID_ARG, "T8 recv NULL can" );
    CHECK( can_recv( FDCAN1, NULL, 1U ) == RES_ERR_INVALID_ARG, "T8 recv NULL frame" );
    CHECK( can_send( NULL, &tx ) == RES_ERR_INVALID_ARG, "T8 send NULL can" );
    CHECK( can_send( FDCAN1, NULL ) == RES_ERR_INVALID_ARG, "T8 send NULL frame" );

    uint32_t const start = delay_get_tick();
    result_t const r = can_recv( FDCAN1, &rx, 15U );
    uint32_t const elapsed = delay_get_tick() - start;

    CHECK( r == RES_ERR_TIMEOUT, "T8 empty recv times out" );
    CHECK( ( elapsed >= 15U ) && ( elapsed <= 30U ), "T8 timeout duration ~15 ms" );
}

/* ------------------------------------------------------------------ */
/* ISO-TP over the loopback                                           */
/* ------------------------------------------------------------------ */

static void test_9_cantp_sf_classic( void )
{
    can_tp_ctx_t ctx;
    can_tp_init( &ctx );
    ctx.use_fd = false;

    uint8_t payload[ 5U ] = { 0x01U, 0x02U, 0x03U, 0x04U, 0x05U };
    uint8_t rxbuf[ 32U ];
    uint32_t out_len = 0U;

    result_t sr = can_tp_send( &ctx, 0x7E8U, payload, sizeof( payload ) );
    CHECK( RES_IS_OK( sr ), "T9 can_tp_send" );

    result_t rr = can_tp_recv( &ctx, rxbuf, sizeof( rxbuf ), &out_len, 20U );
    CHECK( RES_IS_OK( rr ), "T9 can_tp_recv" );
    CHECK( out_len == sizeof( payload ), "T9 out_len" );
    CHECK( mem_equal( payload, rxbuf, sizeof( payload ) ), "T9 payload match" );
}

static void test_10_cantp_sf_fd( void )
{
    can_tp_ctx_t ctx;
    can_tp_init( &ctx );
    ctx.use_fd = true;

    uint8_t payload[ 50U ];
    for( uint32_t i = 0U; i < sizeof( payload ); ++i ) { payload[ i ] = ( uint8_t )( i * 3U ); }

    uint8_t rxbuf[ 64U ];
    uint32_t out_len = 0U;

    result_t sr = can_tp_send( &ctx, 0x7E8U, payload, sizeof( payload ) );
    CHECK( RES_IS_OK( sr ), "T10 can_tp_send" );

    result_t rr = can_tp_recv( &ctx, rxbuf, sizeof( rxbuf ), &out_len, 20U );
    CHECK( RES_IS_OK( rr ), "T10 can_tp_recv" );
    CHECK( out_len == sizeof( payload ), "T10 out_len" );
    CHECK( mem_equal( payload, rxbuf, sizeof( payload ) ), "T10 payload match" );
}

/**
 * Multi-frame receive. The frames are injected with can_send() (the loopback
 * "ECU"); can_tp_recv() answers the First Frame with Flow Control on 0x7DF,
 * which the RX filter (0x7E8..0x7EF) discards, so it does not pollute the FIFO.
 */
static void test_11_cantp_multiframe_recv( void )
{
    can_tp_ctx_t ctx;
    can_tp_init( &ctx );
    ctx.use_fd = false;

    uint8_t expect[ 20U ];
    for( uint32_t i = 0U; i < sizeof( expect ); ++i ) { expect[ i ] = ( uint8_t )( 0xA0U + i ); }

    can_frame_t ff = make_classic( 0x7E8U, 0U );
    ff.data[ 0 ] = 0x10U;
    ff.data[ 1 ] = ( uint8_t )sizeof( expect );
    mem_copy( &ff.data[ 2 ], &expect[ 0 ], 6U );

    can_frame_t cf1 = make_classic( 0x7E8U, 0U );
    cf1.data[ 0 ] = 0x21U;
    mem_copy( &cf1.data[ 1 ], &expect[ 6 ], 7U );

    can_frame_t cf2 = make_classic( 0x7E8U, 0U );
    cf2.data[ 0 ] = 0x22U;
    mem_copy( &cf2.data[ 1 ], &expect[ 13 ], 7U );

    CHECK( RES_IS_OK( can_send( FDCAN1, &ff ) ), "T11 send FF" );
    CHECK( RES_IS_OK( can_send( FDCAN1, &cf1 ) ), "T11 send CF1" );
    CHECK( RES_IS_OK( can_send( FDCAN1, &cf2 ) ), "T11 send CF2" );
    CHECK( wait_tx_idle(), "T11 TX completes" );

    uint8_t rxbuf[ 64U ];
    uint32_t out_len = 0U;
    result_t const rr = can_tp_recv( &ctx, rxbuf, sizeof( rxbuf ), &out_len, 20U );

    CHECK( RES_IS_OK( rr ), "T11 can_tp_recv multi-frame" );
    CHECK( out_len == sizeof( expect ), "T11 reassembled length" );
    CHECK( mem_equal( expect, rxbuf, sizeof( expect ) ), "T11 reassembled data" );
    CHECK( wait_tx_idle(), "T11 flow control transmitted" );
    CHECK( ( FDCAN1->RXF0S & FDCAN_RXF0S_F0FL_Msk ) == 0U, "T11 flow control filtered out of RX" );
}

static void test_12_cantp_multiframe_send_needs_flow_control( void )
{
    can_tp_ctx_t ctx;
    can_tp_init( &ctx );
    ctx.use_fd = false;

    uint8_t payload[ 20U ];
    mem_set( payload, 0x5AU, sizeof( payload ) );

    /*
     * Loopback echoes our own First Frame, which is not a Flow Control frame:
     * the sender must reject it rather than carry on.
     */
    result_t r = can_tp_send( &ctx, 0x7E8U, payload, sizeof( payload ) );
    CHECK( r == RES_ERR_PROTOCOL, "T12 echoed FF is not FC -> PROTOCOL" );
    drain_rx_fifo();

    /*
     * Sent on an ID the RX filter drops, so no reply can ever arrive: the
     * sender must give up on the FC wait instead of hanging.
     */
    uint32_t const start = delay_get_tick();
    r = can_tp_send( &ctx, 0x7DFU, payload, sizeof( payload ) );
    uint32_t const elapsed = delay_get_tick() - start;

    CHECK( r == RES_ERR_TIMEOUT, "T12 no FC -> TIMEOUT" );
    CHECK( elapsed < 500U, "T12 FC wait is bounded" );
    drain_rx_fifo();
}

/* ------------------------------------------------------------------ */
/* CAN FD bit-rate switching (re-initialises the peripheral)          */
/* ------------------------------------------------------------------ */

static void test_13_brs( void )
{
    result_t const ir = init_loopback_with( TEST_DBTP );
    CHECK( RES_IS_OK( ir ), "T13 init with data timing" );

    can_frame_t tx = { .id = 0x7E8U, .dlc = CAN_FD_DLC_64, .fd = true, .brs = true };
    can_frame_t rx;

    for( uint32_t i = 0U; i < CAN_FRAME_FD_DATA_LEN; ++i )
    {
        tx.data[ i ] = ( uint8_t )( 0xFFU - i );
    }

    CHECK( RES_IS_OK( can_send( FDCAN1, &tx ) ), "T13 send BRS frame" );
    result_t const rr = can_recv( FDCAN1, &rx, 10U );

    CHECK( RES_IS_OK( rr ), "T13 recv BRS frame" );
    CHECK( rx.fd == true, "T13 fd flag" );
    CHECK( rx.brs == true, "T13 brs flag preserved" );
    CHECK( mem_equal( tx.data, rx.data, CAN_FRAME_FD_DATA_LEN ), "T13 data" );
}

/* ------------------------------------------------------------------ */

/** Anchor for scripts: break here, then read hitl_pass/hitl_fail/hitl_fails. */
void __attribute__(( noinline )) hitl_finish( void )
{
    hitl_done = 1U;
    __asm volatile( "" ::: "memory" );
}

uint32_t run_can_loopback_tests( void )
{
    LOGI( TAG, "---------- CAN loopback HITL starting ----------" );

    result_t const init_res = init_loopback_with( 0U );
    CHECK( RES_IS_OK( init_res ), "init: can_init_loopback" );

    if( RES_IS_OK( init_res ) )
    {
        test_1_classic_sf();                        drain_rx_fifo();
        test_2_fd_64();                             drain_rx_fifo();
        test_3_dlc_sweep();                         drain_rx_fifo();
        test_4_id_filter();                         drain_rx_fifo();
        test_5_tx_fifo_full();
        test_6_rx_overflow();
        test_7_ordering();                          drain_rx_fifo();
        test_8_recv_edge_cases();
        test_9_cantp_sf_classic();                  drain_rx_fifo();
        test_10_cantp_sf_fd();                      drain_rx_fifo();
        test_11_cantp_multiframe_recv();            drain_rx_fifo();
        test_12_cantp_multiframe_send_needs_flow_control();
        test_13_brs();                              drain_rx_fifo();
    }

    LOGI_U32( TAG, "PASS: ", hitl_pass );
    LOGI_U32( TAG, "FAIL: ", hitl_fail );
    LOGI( TAG, "---------- CAN loopback HITL done ----------" );

    return hitl_fail;
}
