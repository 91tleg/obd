/**
 * @file test_obd_protocol.c
 * @brief SIL tests for OBD-II protocol layer.
 *
 * Tests the full stack from obd_protocol_poll() down through can_tp to
 * the CAN stub.
 */

#include "unity.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "can_stub.h"
#include "ecu_sim.h"
#include "lib/core/result.h"
#include "drivers/protocol/obd/obd_protocol.h"
#include "drivers/protocol/obd/obd_types.h"

void setUp( void )
{
    stub_reset();
    ecu_sim_reset();
    ecu_sim_enable( true );
    stub_set_on_send( ecu_sim_process );
}

void tearDown( void )
{
    /* Nothing to teardown */
}

static obd_protocol_ctx_t * init_ctx( void )
{
    obd_protocol_ctx_t * ctx = NULL;
    result_t r = obd_protocol_init( &ctx );
    TEST_ASSERT_EQUAL( RES_OK, r );
    TEST_ASSERT_NOT_NULL( ctx );
    return ctx;
}

void test_init_returns_ok( void )
{
    obd_protocol_ctx_t * ctx = NULL;
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_init( &ctx ) );
    TEST_ASSERT_NOT_NULL( ctx );
}

void test_init_null_returns_invalid_arg( void )
{
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG, obd_protocol_init( NULL ) );
}

void test_keep_alive_returns_ok( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_keep_alive( ctx ) );
}

void test_set_pids_returns_ok( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    uint8_t pids[2] = { 0x0DU, 0x05U };
    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_set_pids( ctx, pids, 2U ) );
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
    uint8_t pids[1] = { 0x0DU };
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_set_pids( ctx, pids, 0U ) );
}

void test_poll_vehicle_speed_80kmh( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /* ECU responds: speed = 80 km/h (raw byte = 80) */
    uint8_t data[1] = { 80U };
    ecu_sim_register_pid( 0x01U, 0x0DU, data, 1U );

    uint8_t pids[1] = { 0x0DU };
    obd_protocol_set_pids( ctx, pids, 1U );

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK,
                       obd_protocol_poll( ctx, &out, 0U ) );
    TEST_ASSERT_EQUAL( 1U, out.count );
    TEST_ASSERT_TRUE( out.params[0].valid );
}

void test_poll_vehicle_speed_zero( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    uint8_t data[1] = { 0U };
    ecu_sim_register_pid( 0x01U, 0x0DU, data, 1U );

    uint8_t pids[1] = { 0x0DU };
    obd_protocol_set_pids( ctx, pids, 1U );

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK,
                       obd_protocol_poll( ctx, &out, 0U ) );
    TEST_ASSERT_EQUAL( 1U, out.count );
    TEST_ASSERT_TRUE( out.params[0].valid );
}

void test_poll_coolant_temp( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /* ECU responds: temp = 110 raw (70C after -40 offset) */
    uint8_t data[1] = { 110U };
    ecu_sim_register_pid( 0x01U, 0x05U, data, 1U );

    uint8_t pids[1] = { 0x05U };
    obd_protocol_set_pids( ctx, pids, 1U );

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK,
                       obd_protocol_poll( ctx, &out, 0U ) );
    TEST_ASSERT_EQUAL( 1U, out.count );
    TEST_ASSERT_TRUE( out.params[0].valid );
}

void test_poll_engine_rpm_3000( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /* RPM 3000 = 0x0BB8 raw (A*256+B)/4 = 3000 -> A=0x0B B=0xB8 */
    uint8_t data[2] = { 0x0BU, 0xB8U };
    ecu_sim_register_pid( 0x01U, 0x0CU, data, 2U );

    uint8_t pids[1] = { 0x0CU };
    obd_protocol_set_pids( ctx, pids, 1U );

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK,
                       obd_protocol_poll( ctx, &out, 0U ) );
    TEST_ASSERT_EQUAL( 1U, out.count );
    TEST_ASSERT_TRUE( out.params[0].valid );
}

void test_poll_multiple_pids( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    uint8_t speed[1]  = { 60U };
    uint8_t temp[1]   = { 90U };
    uint8_t rpm[2]    = { 0x05U, 0xDCU };   /* 1500 RPM */

    ecu_sim_register_pid( 0x01U, 0x0DU, speed, 1U );
    ecu_sim_register_pid( 0x01U, 0x05U, temp,  1U );
    ecu_sim_register_pid( 0x01U, 0x0CU, rpm,   2U );

    uint8_t pids[3] = { 0x0DU, 0x05U, 0x0CU };
    obd_protocol_set_pids( ctx, pids, 3U );

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_OK,
                       obd_protocol_poll( ctx, &out, 0U ) );
    TEST_ASSERT_EQUAL( 3U, out.count );
    TEST_ASSERT_TRUE( out.params[0].valid );
    TEST_ASSERT_TRUE( out.params[1].valid );
    TEST_ASSERT_TRUE( out.params[2].valid );
}

void test_poll_no_pids_returns_no_data( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    /* no set_pids called */

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_ERR_NO_DATA,
                       obd_protocol_poll( ctx, &out, 0U ) );
}

void test_poll_negative_response_skips_pid( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /* PID 0x0D returns negative, PID 0x05 returns valid */
    ecu_sim_register_negative( 0x01U, 0x0DU );
    uint8_t temp[1] = { 100U };
    ecu_sim_register_pid( 0x01U, 0x05U, temp, 1U );

    uint8_t pids[2] = { 0x0DU, 0x05U };
    obd_protocol_set_pids( ctx, pids, 2U );

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    /* Negative response skips that PID */
    result_t r = obd_protocol_poll( ctx, &out, 0U );
    TEST_ASSERT_EQUAL( RES_OK, r );
}

void test_poll_timeout_when_no_ecu_response( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /* disable ECU sim — no responses queued */
    ecu_sim_enable( false );

    uint8_t pids[1] = { 0x0DU };
    obd_protocol_set_pids( ctx, pids, 1U );

    live_data_t out;
    memset( &out, 0, sizeof( out ) );

    result_t r = obd_protocol_poll( ctx, &out, 0U );
    TEST_ASSERT_EQUAL( RES_ERR_TIMEOUT, r );
}

void test_poll_null_ctx_returns_invalid_arg( void )
{
    live_data_t out;
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_poll( NULL, &out, 0U ) );
}

void test_poll_null_out_returns_invalid_arg( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_poll( ctx, NULL, 0U ) );
}

void test_read_dtcs_null_ctx_returns_invalid_arg( void )
{
    dtc_list_t out;
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_read_dtcs( NULL, &out ) );
}

void test_read_dtcs_null_out_returns_invalid_arg( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_read_dtcs( ctx, NULL ) );
}

void test_read_dtcs_no_dtcs_returns_no_data( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /*
     * Simulate ECU response to Mode 03 with no DTCs:
     * response byte[0] = 0x43 (Mode 03 positive), byte[1] = 0x00 (0 DTCs)
     */
    can_frame_t resp;
    memset( &resp, 0, sizeof( resp ) );
    resp.id      = 0x7E8U;
    resp.data[0] = 0x02U;   /* SF, 2 bytes */
    resp.data[1] = 0x43U;   /* Mode 03 positive response */
    resp.data[2] = 0x00U;   /* 0 DTCs */
    stub_queue_recv( &resp );

    /* Mode 07 (pending), also no DTCs */
    stub_queue_recv( &resp );

    dtc_list_t out;
    memset( &out, 0, sizeof( out ) );

    TEST_ASSERT_EQUAL( RES_ERR_NO_DATA,
                       obd_protocol_read_dtcs( ctx, &out ) );
    TEST_ASSERT_EQUAL( 0U, out.count );
}

void test_clear_dtcs_null_ctx_returns_invalid_arg( void )
{
    TEST_ASSERT_EQUAL( RES_ERR_INVALID_ARG,
                       obd_protocol_clear_dtcs( NULL ) );
}

void test_clear_dtcs_ok_response( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /* Mode 04 positive response: 0x44 */
    can_frame_t resp;
    memset( &resp, 0, sizeof( resp ) );
    resp.id      = 0x7E8U;
    resp.data[0] = 0x01U;   /* SF, 1 byte */
    resp.data[1] = 0x44U;   /* Mode 04 positive */
    stub_queue_recv( &resp );

    TEST_ASSERT_EQUAL( RES_OK, obd_protocol_clear_dtcs( ctx ) );
}

void test_clear_dtcs_wrong_response_returns_protocol_err( void )
{
    obd_protocol_ctx_t * ctx = init_ctx();

    /* Wrong response byte */
    can_frame_t resp;
    memset( &resp, 0, sizeof( resp ) );
    resp.id      = 0x7E8U;
    resp.data[0] = 0x01U;
    resp.data[1] = 0x41U;   /* Mode 01 response, not Mode 04 */
    stub_queue_recv( &resp );

    TEST_ASSERT_EQUAL( RES_ERR_PROTOCOL, obd_protocol_clear_dtcs( ctx ) );
}

int main( void )
{
    UNITY_BEGIN();

    /* init */
    RUN_TEST( test_init_returns_ok );
    RUN_TEST( test_init_null_returns_invalid_arg );

    /* keep alive */
    RUN_TEST( test_keep_alive_returns_ok );

    /* set pids */
    RUN_TEST( test_set_pids_returns_ok );
    RUN_TEST( test_set_pids_null_pids_returns_invalid_arg );
    RUN_TEST( test_set_pids_zero_count_returns_invalid_arg );

    /* poll single PIDs */
    RUN_TEST( test_poll_vehicle_speed_80kmh );
    RUN_TEST( test_poll_vehicle_speed_zero );
    RUN_TEST( test_poll_coolant_temp );
    RUN_TEST( test_poll_engine_rpm_3000 );

    /* poll multiple PIDs */
    RUN_TEST( test_poll_multiple_pids );

    /* poll error cases */
    RUN_TEST( test_poll_no_pids_returns_no_data );
    RUN_TEST( test_poll_negative_response_skips_pid );
    RUN_TEST( test_poll_timeout_when_no_ecu_response );
    RUN_TEST( test_poll_null_ctx_returns_invalid_arg );
    RUN_TEST( test_poll_null_out_returns_invalid_arg );

    /* read DTCs */
    RUN_TEST( test_read_dtcs_null_ctx_returns_invalid_arg );
    RUN_TEST( test_read_dtcs_null_out_returns_invalid_arg );
    RUN_TEST( test_read_dtcs_no_dtcs_returns_no_data );

    /* clear DTCs */
    RUN_TEST( test_clear_dtcs_null_ctx_returns_invalid_arg );
    RUN_TEST( test_clear_dtcs_ok_response );
    RUN_TEST( test_clear_dtcs_wrong_response_returns_protocol_err );

    return UNITY_END();
}
