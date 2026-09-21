/**
 * @file test_obd_protocol.c
 * @brief SIL tests for the OBD-II over CAN protocol layer.
 *
 * The real obd_protocol / protocol / decoder / can_tp code runs unmodified.
 * Only the CAN HAL is replaced: can_stub captures every transmitted frame and
 * ecu_sim answers like an ISO 15765-2 ECU (including Flow Control handshakes).
 *
 * Tests check both directions of the bus: what the tester puts on the wire
 * (request frames, flow control) and what it does with the replies (decoded
 * engineering values, error codes).
 */

#include "unity.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "can_stub.h"
#include "ecu_sim.h"
#include "lib/core/result.h"
#include "drivers/protocol/obd/cmd.h"
#include "drivers/protocol/obd/obd_protocol.h"
#include "drivers/protocol/obd/obd_types.h"

#define REQ_ID          ( 0x7DFU )
#define FLOAT_TOL       ( 0.05F )
#define TIMESTAMP       ( 1234U )

static uint32_t s_delay_calls;

void delay_ms( uint32_t ms )
{
    ( void )ms;
    ++s_delay_calls;
}

void setUp( void )
{
    stub_reset();
    ecu_sim_reset();
    ecu_sim_enable( true );
    stub_set_on_send( ecu_sim_process );
    s_delay_calls = 0U;
}

void tearDown( void )
{
    /* A harness overflow means frames were silently lost: test is invalid. */
    TEST_ASSERT_EQUAL_HEX32_MESSAGE( 0U, stub_errors(),
                                     "CAN stub queue overflowed" );
}

static obd_protocol_ctx_t * init_ctx( void )
{
    obd_protocol_ctx_t * ctx = NULL;
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_init( &ctx ) );
    TEST_ASSERT_NOT_NULL( ctx );
    return ctx;
}

/** Assert sent frame [idx] is a single-frame request carrying payload. */
static void assert_tx_sf( uint32_t idx, uint8_t const * payload, uint8_t len )
{
    can_frame_t const * f = stub_sent( idx );

    TEST_ASSERT_NOT_NULL_MESSAGE( f, "expected TX frame is missing" );
    TEST_ASSERT_EQUAL_HEX32( REQ_ID, f->id );
    TEST_ASSERT_FALSE( f->fd );
    TEST_ASSERT_EQUAL_HEX8( len, f->data[ 0 ] );   /* PCI 0x0n */
    TEST_ASSERT_EQUAL_HEX8_ARRAY( payload, &f->data[ 1 ], len );
}

static void assert_tx_pid_request( uint32_t idx, uint8_t pid )
{
    uint8_t const req[ 2 ] = { OBD_MODE_CURRENT_DATA, pid };
    assert_tx_sf( idx, req, 2U );
}

static void assert_tx_mode_request( uint32_t idx, uint8_t mode )
{
    assert_tx_sf( idx, &mode, 1U );
}

static live_data_param_t const * find_param( live_data_t const * d,
                                             obd_param_id_t id )
{
    for( uint32_t i = 0U; i < d->count; ++i )
    {
        if( d->params[ i ].id == id )
        {
            return &d->params[ i ];
        }
    }
    return NULL;
}

static result_t poll_pids( obd_protocol_ctx_t * ctx,
                           uint8_t const * pids,
                           uint32_t n,
                           live_data_t * out )
{
    memset( out, 0, sizeof( *out ) );
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_set_pids( ctx, pids, n ) );
    return obd_protocol_poll( ctx, out, TIMESTAMP );
}

void test_init_returns_ok( void )
{
    ( void )init_ctx();
}

void test_init_null_returns_invalid_arg( void )
{
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_init( NULL ) );
}

void test_init_clears_previously_configured_pids( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    live_data_t out;

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_set_pids( ctx, pids, 1U ) );

    ctx = init_ctx();
    memset( &out, 0, sizeof( out ) );
    TEST_ASSERT_EQUAL( RES_ERR_NO_DATA, obd_protocol_poll( ctx, &out, 0U ) );
}

void test_keep_alive_is_a_nop_on_the_bus( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_keep_alive( ctx ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, stub_sent_count() );
}

void test_set_pids_accepts_valid_list( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 2 ] = { OBD_PID_VEHICLE_SPEED, OBD_PID_COOLANT_TEMP };

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_set_pids( ctx, pids, 2U ) );
}

void test_set_pids_null_ctx_returns_invalid_arg( void )
{
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_set_pids( NULL, pids, 1U ) );
}

void test_set_pids_null_pids_returns_invalid_arg( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_set_pids( ctx, NULL, 1U ) );
}

void test_set_pids_zero_count_returns_invalid_arg( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_set_pids( ctx, pids, 0U ) );
}

void test_set_pids_too_many_returns_invalid_arg( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 33 ];

    memset( pids, OBD_PID_VEHICLE_SPEED, sizeof( pids ) );
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_set_pids( ctx, pids, 32U ) );
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_set_pids( ctx, pids, 33U ) );
}

typedef struct
{
    uint8_t        pid;
    uint8_t        data[ 2 ];
    uint8_t        len;
    obd_param_id_t id;
    float          expected;
} pid_vector_t;

/* Expected values follow SAE J1979 Table B.x scaling. */
static pid_vector_t const s_vectors[] =
{
    { OBD_PID_ENGINE_LOAD,     { 0xFFU },        1U, OBD_PARAM_ENGINE_LOAD,       100.0F   },
    { OBD_PID_ENGINE_LOAD,     { 0x00U },        1U, OBD_PARAM_ENGINE_LOAD,       0.0F     },
    { OBD_PID_COOLANT_TEMP,    { 110U },         1U, OBD_PARAM_COOLANT_TEMP,      70.0F    },
    { OBD_PID_COOLANT_TEMP,    { 0U },           1U, OBD_PARAM_COOLANT_TEMP,      -40.0F   },
    { OBD_PID_COOLANT_TEMP,    { 255U },         1U, OBD_PARAM_COOLANT_TEMP,      215.0F   },
    { OBD_PID_SHORT_TERM_FUEL, { 128U },         1U, OBD_PARAM_SHORT_TERM_FUEL,   0.0F     },
    { OBD_PID_INTAKE_PRESSURE, { 101U },         1U, OBD_PARAM_INTAKE_PRESSURE,   101.0F   },
    { OBD_PID_RPM,             { 0x2EU, 0xE0U }, 2U, OBD_PARAM_RPM,               3000.0F  },
    { OBD_PID_RPM,             { 0x00U, 0x00U }, 2U, OBD_PARAM_RPM,               0.0F     },
    { OBD_PID_RPM,             { 0x00U, 0x01U }, 2U, OBD_PARAM_RPM,               0.25F    },
    { OBD_PID_RPM,             { 0xFFU, 0xFFU }, 2U, OBD_PARAM_RPM,               16383.75F },
    { OBD_PID_VEHICLE_SPEED,   { 80U },          1U, OBD_PARAM_VEHICLE_SPEED,     80.0F    },
    { OBD_PID_VEHICLE_SPEED,   { 0U },           1U, OBD_PARAM_VEHICLE_SPEED,     0.0F     },
    { OBD_PID_VEHICLE_SPEED,   { 255U },         1U, OBD_PARAM_VEHICLE_SPEED,     255.0F   },
    { OBD_PID_TIMING_ADVANCE,  { 128U },         1U, OBD_PARAM_TIMING_ADVANCE,    0.0F     },
    { OBD_PID_TIMING_ADVANCE,  { 0U },           1U, OBD_PARAM_TIMING_ADVANCE,    -64.0F   },
    { OBD_PID_INTAKE_TEMP,     { 65U },          1U, OBD_PARAM_INTAKE_TEMP,       25.0F    },
    { OBD_PID_MAF,             { 0x0FU, 0xA0U }, 2U, OBD_PARAM_MAF,               40.0F    },
    { OBD_PID_THROTTLE,        { 51U },          1U, OBD_PARAM_THROTTLE,          20.0F    },
    { OBD_PID_RUNTIME,         { 0x01U, 0x2CU }, 2U, OBD_PARAM_RUNTIME,           300.0F   },
    { OBD_PID_DISTANCE_MIL,    { 0x00U, 0x64U }, 2U, OBD_PARAM_DISTANCE_MIL,      100.0F   },
    { OBD_PID_FUEL_LEVEL,      { 128U },         1U, OBD_PARAM_FUEL_LEVEL,        50.2F    },
    { OBD_PID_BAROMETRIC,      { 101U },         1U, OBD_PARAM_BAROMETRIC,        101.0F   },
    { OBD_PID_CONTROL_MODULE,  { 0x35U, 0x84U }, 2U, OBD_PARAM_CONTROL_MODULE_V,  13.7F    },
    { OBD_PID_AMBIENT_TEMP,    { 60U },          1U, OBD_PARAM_AMBIENT_TEMP,      20.0F    },
    { OBD_PID_OIL_TEMP,        { 130U },         1U, OBD_PARAM_OIL_TEMP,          90.0F    },
    { OBD_PID_FUEL_RATE,       { 0x00U, 0xC8U }, 2U, OBD_PARAM_FUEL_RATE,         10.0F    },
};

void test_poll_decodes_every_supported_pid( void )
{
    for( uint32_t i = 0U; i < ( sizeof( s_vectors ) / sizeof( s_vectors[ 0 ] ) ); ++i )
    {
        pid_vector_t const * v = &s_vectors[ i ];
        char msg[ 48 ];
        live_data_t out;

        /* Fresh bus and ECU for each vector so failures are independent. */
        setUp();
        obd_protocol_ctx_t * ctx = init_ctx();

        ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, v->pid, v->data, v->len );

        ( void )snprintf( msg, sizeof( msg ), "vector %u pid 0x%02X",
                          ( unsigned )i, ( unsigned )v->pid );

        TEST_ASSERT_EQUAL_MESSAGE( RES_OK, poll_pids( ctx, &v->pid, 1U, &out ), msg );
        TEST_ASSERT_EQUAL_UINT32_MESSAGE( 1U, out.count, msg );
        TEST_ASSERT_TRUE_MESSAGE( out.params[ 0 ].valid, msg );
        TEST_ASSERT_EQUAL_UINT32_MESSAGE( v->id, out.params[ 0 ].id, msg );
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE( FLOAT_TOL, v->expected,
                                          out.params[ 0 ].value, msg );
    }
}

void test_poll_stamps_timestamp_on_each_param( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t spd[ 1 ] = { 50U };
    uint8_t tmp[ 1 ] = { 90U };
    uint8_t pids[ 2 ] = { OBD_PID_VEHICLE_SPEED, OBD_PID_COOLANT_TEMP };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_COOLANT_TEMP, tmp, 1U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 2U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 2U, out.count );
    TEST_ASSERT_EQUAL_UINT32( TIMESTAMP, out.params[ 0 ].timestamp );
    TEST_ASSERT_EQUAL_UINT32( TIMESTAMP, out.params[ 1 ].timestamp );
}

void test_poll_sends_one_request_per_pid_in_order( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t spd[ 1 ] = { 60U };
    uint8_t tmp[ 1 ] = { 90U };
    uint8_t rpm[ 2 ] = { 0x17U, 0x70U };   /* 1500 RPM = 6000 / 4 */
    uint8_t pids[ 3 ] = { OBD_PID_VEHICLE_SPEED, OBD_PID_COOLANT_TEMP, OBD_PID_RPM };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_COOLANT_TEMP, tmp, 1U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_RPM, rpm, 2U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 3U, &out ) );

    TEST_ASSERT_EQUAL_UINT32( 3U, stub_sent_count() );
    assert_tx_pid_request( 0U, OBD_PID_VEHICLE_SPEED );
    assert_tx_pid_request( 1U, OBD_PID_COOLANT_TEMP );
    assert_tx_pid_request( 2U, OBD_PID_RPM );
    TEST_ASSERT_EQUAL_UINT32( 0U, stub_recv_remaining() );

    TEST_ASSERT_EQUAL_UINT32( 3U, out.count );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 60.0F,
        find_param( &out, OBD_PARAM_VEHICLE_SPEED )->value );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 50.0F,
        find_param( &out, OBD_PARAM_COOLANT_TEMP )->value );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 1500.0F,
        find_param( &out, OBD_PARAM_RPM )->value );
}

void test_poll_twice_reflects_updated_ecu_values( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    uint8_t v1[ 1 ] = { 80U };
    uint8_t v2[ 1 ] = { 100U };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, v1, 1U );
    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 80.0F, out.params[ 0 ].value );

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, v2, 1U );
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_poll( ctx, &out, TIMESTAMP + 100U ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, out.count );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 100.0F, out.params[ 0 ].value );
    TEST_ASSERT_EQUAL_UINT32( TIMESTAMP + 100U, out.params[ 0 ].timestamp );
}

void test_poll_multi_frame_response_sends_flow_control_and_decodes( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t data[ 8 ] = { 0x2EU, 0xE0U, 1U, 2U, 3U, 4U, 5U, 6U };   /* 10 B reply */
    uint8_t pids[ 1 ] = { OBD_PID_RPM };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_RPM, data, 8U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, out.count );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 3000.0F, out.params[ 0 ].value );

    /* Request, then Flow Control CTS from the tester. */
    TEST_ASSERT_EQUAL_UINT32( 2U, stub_sent_count() );
    assert_tx_pid_request( 0U, OBD_PID_RPM );
    TEST_ASSERT_EQUAL_UINT32( 1U, ecu_sim_fc_count() );
    TEST_ASSERT_EQUAL_HEX8( 0x30U, stub_sent( 1U )->data[ 0 ] );
    TEST_ASSERT_EQUAL_UINT32( 0U, stub_recv_remaining() );
}

void test_poll_long_response_wraps_consecutive_frame_sequence_number( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t data[ 120 ];
    uint8_t pids[ 1 ] = { OBD_PID_RPM };
    live_data_t out;

    /* 122-byte payload -> 17 CFs, SN runs 1..15,0,1 */
    memset( data, 0x5AU, sizeof( data ) );
    data[ 0 ] = 0x2EU;
    data[ 1 ] = 0xE0U;
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_RPM, data, 120U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, out.count );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 3000.0F, out.params[ 0 ].value );
    TEST_ASSERT_EQUAL_UINT32( 0U, stub_recv_remaining() );
}

void test_poll_bad_consecutive_frame_sn_returns_protocol_error( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t data[ 30 ];
    uint8_t pids[ 1 ] = { OBD_PID_RPM };
    live_data_t out;

    memset( data, 0x11U, sizeof( data ) );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_RPM, data, 30U );
    ecu_sim_set_fault( ECU_SIM_FAULT_BAD_SN );

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL, poll_pids( ctx, pids, 1U, &out ) );
}

void test_poll_missing_consecutive_frames_returns_timeout( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t data[ 30 ];
    uint8_t pids[ 1 ] = { OBD_PID_RPM };
    live_data_t out;

    memset( data, 0x11U, sizeof( data ) );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_RPM, data, 30U );
    ecu_sim_set_fault( ECU_SIM_FAULT_DROP_CFS );

    TEST_ASSERT_EQUAL( RES_ERR_TIMEOUT, poll_pids( ctx, pids, 1U, &out ) );
}

void test_poll_no_pids_returns_no_data_and_sends_nothing( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    live_data_t out;

    memset( &out, 0, sizeof( out ) );
    TEST_ASSERT_EQUAL( RES_ERR_NO_DATA, obd_protocol_poll( ctx, &out, 0U ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, stub_sent_count() );
}

void test_poll_null_ctx_returns_invalid_arg( void )
{
    live_data_t out;
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_poll( NULL, &out, 0U ) );
}

void test_poll_null_out_returns_invalid_arg( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_poll( ctx, NULL, 0U ) );
}

void test_poll_negative_response_skips_only_that_pid( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t temp[ 1 ] = { 100U };
    uint8_t pids[ 2 ] = { OBD_PID_VEHICLE_SPEED, OBD_PID_COOLANT_TEMP };
    live_data_t out;

    ecu_sim_register_negative( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_COOLANT_TEMP, temp, 1U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 2U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, out.count );
    TEST_ASSERT_NULL( find_param( &out, OBD_PARAM_VEHICLE_SPEED ) );
    TEST_ASSERT_NOT_NULL( find_param( &out, OBD_PARAM_COOLANT_TEMP ) );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 60.0F,
        find_param( &out, OBD_PARAM_COOLANT_TEMP )->value );
    TEST_ASSERT_EQUAL_UINT32( 2U, stub_sent_count() );
}

void test_poll_silent_ecu_returns_timeout( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    live_data_t out;

    ecu_sim_enable( false );

    TEST_ASSERT_EQUAL( RES_ERR_TIMEOUT, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, stub_sent_count() );
    assert_tx_pid_request( 0U, OBD_PID_VEHICLE_SPEED );
}

void test_poll_timeout_mid_list_aborts_remaining_pids( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t spd[ 1 ] = { 50U };
    uint8_t pids[ 3 ] = { OBD_PID_VEHICLE_SPEED, OBD_PID_COOLANT_TEMP, OBD_PID_RPM };
    live_data_t out;

    /* coolant temp and RPM are not registered => ECU silent on the 2nd PID */
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );

    TEST_ASSERT_EQUAL( RES_ERR_TIMEOUT, poll_pids( ctx, pids, 3U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 2U, stub_sent_count() );   /* never asked for RPM */
}

void test_poll_bus_error_is_propagated( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t spd[ 1 ] = { 50U };
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );
    stub_set_recv_error( RES_ERR_BUS );

    TEST_ASSERT_EQUAL( RES_ERR_BUS, poll_pids( ctx, pids, 1U, &out ) );
}

void test_poll_wrong_pid_echo_is_skipped_not_decoded( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t spd[ 1 ] = { 50U };
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );
    ecu_sim_set_fault( ECU_SIM_FAULT_WRONG_PID_ECHO );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, out.count );
}

void test_poll_truncated_response_is_skipped( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    uint8_t truncated[ 1 ] = { 0x41U };   /* mode echo only, no PID / data */
    live_data_t out;

    ecu_sim_inject_sf( ECU_SIM_DEFAULT_RESP_ID, truncated, 1U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, out.count );
}

void test_poll_too_short_data_for_pid_is_skipped( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t one_byte[ 1 ] = { 0x0BU };   /* RPM needs 2 bytes */
    uint8_t pids[ 1 ] = { OBD_PID_RPM };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_RPM, one_byte, 1U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, out.count );
}

void test_poll_pid_unknown_to_decoder_is_skipped( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t data[ 2 ] = { 0x12U, 0x34U };
    uint8_t pids[ 1 ] = { OBD_PID_O2_B1S1 };   /* answered but not decoded */
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_O2_B1S1, data, 2U );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, out.count );
}

void test_poll_response_from_non_obd_id_returns_protocol_error( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t spd[ 1 ] = { 50U };
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );
    ecu_sim_set_fault( ECU_SIM_FAULT_WRONG_ID );

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL, poll_pids( ctx, pids, 1U, &out ) );
}

void test_poll_unknown_pci_type_returns_protocol_error( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    uint8_t bogus[ 3 ] = { 0x50U, 0x41U, 0x0DU };   /* PCI type 5 is reserved */
    live_data_t out;

    ecu_sim_inject_raw( ECU_SIM_DEFAULT_RESP_ID, bogus, 3U );

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL, poll_pids( ctx, pids, 1U, &out ) );
}

void test_poll_accepts_response_from_any_ecu_in_range( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t spd[ 1 ] = { 42U };
    uint8_t pids[ 1 ] = { OBD_PID_VEHICLE_SPEED };
    live_data_t out;

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );
    ecu_sim_set_response_id( 0x7EFU );

    TEST_ASSERT_EQUAL( RES_OK, poll_pids( ctx, pids, 1U, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, out.count );
    TEST_ASSERT_FLOAT_WITHIN( FLOAT_TOL, 42.0F, out.params[ 0 ].value );
}

void test_query_supported_null_ctx_returns_invalid_arg( void )
{
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_query_supported( NULL ) );
}

void test_query_supported_silent_ecu_returns_timeout( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    ecu_sim_enable( false );

    TEST_ASSERT_EQUAL( RES_ERR_TIMEOUT, obd_protocol_query_supported( ctx ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, stub_sent_count() );
    assert_tx_pid_request( 0U, OBD_PID_SUPPORTED_1 );
}

void test_query_supported_walks_groups_and_feeds_poll( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    live_data_t out;

    /*
     * Group 0x00: PIDs 0x05, 0x0C, 0x0D and 0x20 (=> continue to next group).
     *   bit(pid) = 0x80000000 >> (pid - 1)  =>  0x08180001
     * Group 0x20: PIDs 0x21, 0x2F (0x40 not set => stop).
     *                                        =>  0x80020000
     */
    uint8_t g0[ 4 ] = { 0x08U, 0x18U, 0x00U, 0x01U };
    uint8_t g1[ 4 ] = { 0x80U, 0x02U, 0x00U, 0x00U };
    uint8_t temp[ 1 ] = { 100U };
    uint8_t rpm[ 2 ]  = { 0x2EU, 0xE0U };
    uint8_t spd[ 1 ]  = { 55U };
    uint8_t dist[ 2 ] = { 0x00U, 0x0AU };
    uint8_t fuel[ 1 ] = { 255U };

    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_SUPPORTED_1, g0, 4U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_SUPPORTED_2, g1, 4U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_COOLANT_TEMP, temp, 1U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_RPM, rpm, 2U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_VEHICLE_SPEED, spd, 1U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_DISTANCE_MIL, dist, 2U );
    ecu_sim_register_pid( OBD_MODE_CURRENT_DATA, OBD_PID_FUEL_LEVEL, fuel, 1U );

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_query_supported( ctx ) );

    /* Only groups 0x00 and 0x20 were asked for. */
    TEST_ASSERT_EQUAL_UINT32( 2U, stub_sent_count() );
    assert_tx_pid_request( 0U, OBD_PID_SUPPORTED_1 );
    assert_tx_pid_request( 1U, OBD_PID_SUPPORTED_2 );

    /* Poll now uses the discovered list (support PIDs themselves excluded). */
    memset( &out, 0, sizeof( out ) );
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_poll( ctx, &out, TIMESTAMP ) );
    TEST_ASSERT_EQUAL_UINT32( 5U, out.count );
    TEST_ASSERT_EQUAL_UINT32( 7U, stub_sent_count() );
    assert_tx_pid_request( 2U, OBD_PID_COOLANT_TEMP );
    assert_tx_pid_request( 3U, OBD_PID_RPM );
    assert_tx_pid_request( 4U, OBD_PID_VEHICLE_SPEED );
    assert_tx_pid_request( 5U, OBD_PID_DISTANCE_MIL );
    assert_tx_pid_request( 6U, OBD_PID_FUEL_LEVEL );
}

void test_read_dtcs_null_ctx_returns_invalid_arg( void )
{
    dtc_list_t out;
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_read_dtcs( NULL, &out ) );
}

void test_read_dtcs_null_out_returns_invalid_arg( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_read_dtcs( ctx, NULL ) );
}

void test_read_dtcs_none_stored_or_pending_returns_no_data( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t none[ 1 ] = { 0x00U };
    dtc_list_t out;

    ecu_sim_register_mode( OBD_MODE_READ_DTC, none, 1U );
    ecu_sim_register_mode( OBD_MODE_PENDING_DTC, none, 1U );
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_ERR_NO_DATA, obd_protocol_read_dtcs( ctx, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, out.count );
    TEST_ASSERT_EQUAL_UINT32( 2U, stub_sent_count() );
    assert_tx_mode_request( 0U, OBD_MODE_READ_DTC );
    assert_tx_mode_request( 1U, OBD_MODE_PENDING_DTC );
}

void test_read_dtcs_returns_stored_and_pending_with_flags( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t stored[ 5 ]  = { 2U, 0x03U, 0x01U, 0x04U, 0x20U };   /* P0301, P0420 */
    uint8_t pending[ 3 ] = { 1U, 0x01U, 0x71U };                 /* P0171 */
    dtc_list_t out;

    ecu_sim_register_mode( OBD_MODE_READ_DTC, stored, 5U );
    ecu_sim_register_mode( OBD_MODE_PENDING_DTC, pending, 3U );
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_read_dtcs( ctx, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 3U, out.count );

    TEST_ASSERT_EQUAL_HEX16( 0x0301U, out.codes[ 0 ].code );
    TEST_ASSERT_FALSE( out.codes[ 0 ].pending );
    TEST_ASSERT_EQUAL_HEX16( 0x0420U, out.codes[ 1 ].code );
    TEST_ASSERT_FALSE( out.codes[ 1 ].pending );
    TEST_ASSERT_EQUAL_HEX16( 0x0171U, out.codes[ 2 ].code );
    TEST_ASSERT_TRUE( out.codes[ 2 ].pending );
}

void test_read_dtcs_stored_only_when_pending_unanswered( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t stored[ 3 ] = { 1U, 0x03U, 0x01U };
    dtc_list_t out;

    ecu_sim_register_mode( OBD_MODE_READ_DTC, stored, 3U );   /* mode 07 silent */
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_read_dtcs( ctx, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, out.count );
    TEST_ASSERT_EQUAL_HEX16( 0x0301U, out.codes[ 0 ].code );
}

void test_read_dtcs_multi_frame_response_is_reassembled( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t stored[ 1 + ( 5 * 2 ) ];   /* 5 DTCs -> 12 byte payload, needs FF+CF */
    dtc_list_t out;

    stored[ 0 ] = 5U;
    for( uint8_t i = 0U; i < 5U; ++i )
    {
        stored[ 1 + ( i * 2 ) ] = 0x03U;
        stored[ 2 + ( i * 2 ) ] = ( uint8_t )( 0x00U + i + 1U );   /* P0301..P0305 */
    }
    ecu_sim_register_mode( OBD_MODE_READ_DTC, stored, ( uint8_t )sizeof( stored ) );
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_read_dtcs( ctx, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 5U, out.count );
    for( uint16_t i = 0U; i < 5U; ++i )
    {
        TEST_ASSERT_EQUAL_HEX16( 0x0301U + i, out.codes[ i ].code );
    }
    TEST_ASSERT_EQUAL_UINT32( 1U, ecu_sim_fc_count() );
}

/**
 * SAE J1979: DTC top two bits select the system: 00=P, 01=C, 10=B, 11=U.
 */
void test_read_dtcs_decodes_system_letter_from_top_bits( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t stored[ 9 ] = { 4U, 0x01U, 0x23U,    /* P0123 */
                            0x41U, 0x23U,        /* C0123 */
                            0x81U, 0x23U,        /* B0123 */
                            0xC1U, 0x23U };      /* U0123 */
    dtc_list_t out;

    ecu_sim_register_mode( OBD_MODE_READ_DTC, stored, 9U );
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_read_dtcs( ctx, &out ) );
    TEST_ASSERT_EQUAL_UINT32( 4U, out.count );
    TEST_ASSERT_EQUAL( DTC_TYPE_POWERTRAIN, out.codes[ 0 ].type );
    TEST_ASSERT_EQUAL( DTC_TYPE_CHASSIS,    out.codes[ 1 ].type );
    TEST_ASSERT_EQUAL( DTC_TYPE_BODY,       out.codes[ 2 ].type );
    TEST_ASSERT_EQUAL( DTC_TYPE_NETWORK,    out.codes[ 3 ].type );
}

void test_clear_dtcs_null_ctx_returns_invalid_arg( void )
{
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_clear_dtcs( NULL ) );
}

void test_clear_dtcs_positive_response( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    ecu_sim_register_mode( OBD_MODE_CLEAR_DTC, NULL, 0U );   /* reply: 44 */

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_clear_dtcs( ctx ) );
    TEST_ASSERT_EQUAL_UINT32( 1U, stub_sent_count() );
    assert_tx_mode_request( 0U, OBD_MODE_CLEAR_DTC );
}

void test_clear_dtcs_wrong_mode_echo_returns_protocol_error( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t wrong[ 1 ] = { 0x41U };   /* Mode 01 reply to a Mode 04 request */

    ecu_sim_inject_sf( ECU_SIM_DEFAULT_RESP_ID, wrong, 1U );

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL, obd_protocol_clear_dtcs( ctx ) );
}

void test_clear_dtcs_silent_ecu_returns_timeout( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    ecu_sim_enable( false );

    TEST_ASSERT_EQUAL( RES_ERR_TIMEOUT, obd_protocol_clear_dtcs( ctx ) );
}

void test_clear_dtcs_negative_response_fails( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    ecu_sim_register_negative( OBD_MODE_CLEAR_DTC, 0U );

    TEST_ASSERT_NOT_EQUAL( RES_OK, obd_protocol_clear_dtcs( ctx ) );
}

int main( void )
{
    UNITY_BEGIN();

    /* init / keep-alive / set_pids */
    RUN_TEST( test_init_returns_ok );
    RUN_TEST( test_init_null_returns_invalid_arg );
    RUN_TEST( test_init_clears_previously_configured_pids );
    RUN_TEST( test_keep_alive_is_a_nop_on_the_bus );
    RUN_TEST( test_set_pids_accepts_valid_list );
    RUN_TEST( test_set_pids_null_ctx_returns_invalid_arg );
    RUN_TEST( test_set_pids_null_pids_returns_invalid_arg );
    RUN_TEST( test_set_pids_zero_count_returns_invalid_arg );
    RUN_TEST( test_set_pids_too_many_returns_invalid_arg );

    /* poll: decoded values */
    RUN_TEST( test_poll_decodes_every_supported_pid );
    RUN_TEST( test_poll_stamps_timestamp_on_each_param );
    RUN_TEST( test_poll_sends_one_request_per_pid_in_order );
    RUN_TEST( test_poll_twice_reflects_updated_ecu_values );

    /* poll: multi-frame */
    RUN_TEST( test_poll_multi_frame_response_sends_flow_control_and_decodes );
    RUN_TEST( test_poll_long_response_wraps_consecutive_frame_sequence_number );
    RUN_TEST( test_poll_bad_consecutive_frame_sn_returns_protocol_error );
    RUN_TEST( test_poll_missing_consecutive_frames_returns_timeout );

    /* poll: errors */
    RUN_TEST( test_poll_no_pids_returns_no_data_and_sends_nothing );
    RUN_TEST( test_poll_null_ctx_returns_invalid_arg );
    RUN_TEST( test_poll_null_out_returns_invalid_arg );
    RUN_TEST( test_poll_negative_response_skips_only_that_pid );
    RUN_TEST( test_poll_silent_ecu_returns_timeout );
    RUN_TEST( test_poll_timeout_mid_list_aborts_remaining_pids );
    RUN_TEST( test_poll_bus_error_is_propagated );
    RUN_TEST( test_poll_wrong_pid_echo_is_skipped_not_decoded );
    RUN_TEST( test_poll_truncated_response_is_skipped );
    RUN_TEST( test_poll_too_short_data_for_pid_is_skipped );
    RUN_TEST( test_poll_pid_unknown_to_decoder_is_skipped );
    RUN_TEST( test_poll_response_from_non_obd_id_returns_protocol_error );
    RUN_TEST( test_poll_unknown_pci_type_returns_protocol_error );
    RUN_TEST( test_poll_accepts_response_from_any_ecu_in_range );

    /* query_supported */
    RUN_TEST( test_query_supported_null_ctx_returns_invalid_arg );
    RUN_TEST( test_query_supported_silent_ecu_returns_timeout );
    RUN_TEST( test_query_supported_walks_groups_and_feeds_poll );

    /* DTCs */
    RUN_TEST( test_read_dtcs_null_ctx_returns_invalid_arg );
    RUN_TEST( test_read_dtcs_null_out_returns_invalid_arg );
    RUN_TEST( test_read_dtcs_none_stored_or_pending_returns_no_data );
    RUN_TEST( test_read_dtcs_returns_stored_and_pending_with_flags );
    RUN_TEST( test_read_dtcs_stored_only_when_pending_unanswered );
    RUN_TEST( test_read_dtcs_multi_frame_response_is_reassembled );
    RUN_TEST( test_read_dtcs_decodes_system_letter_from_top_bits );
    RUN_TEST( test_clear_dtcs_null_ctx_returns_invalid_arg );
    RUN_TEST( test_clear_dtcs_positive_response );
    RUN_TEST( test_clear_dtcs_wrong_mode_echo_returns_protocol_error );
    RUN_TEST( test_clear_dtcs_silent_ecu_returns_timeout );
    RUN_TEST( test_clear_dtcs_negative_response_fails );

    return UNITY_END();
}
