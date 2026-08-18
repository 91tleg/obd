#include "unity.h"
#include "lib/string/str.h"
#include "lib/log/log.h"

#include <stdint.h>
#include <stdbool.h>

#define CAPTURE_BUF_LEN  ( 256U )

static char s_captured[ CAPTURE_BUF_LEN ];
static uint32_t s_captured_len = 0U;
static uint32_t s_call_count   = 0U;

void log_write_output( const char * buf, uint32_t len )
{
    s_call_count++;
    s_captured_len = ( len < ( CAPTURE_BUF_LEN - 1U ) ) ? len : ( CAPTURE_BUF_LEN - 1U );
    mem_copy( s_captured, buf, s_captured_len );
    s_captured[ s_captured_len ] = '\0';
}

void setUp( void )
{
    mem_zero( s_captured, sizeof( s_captured ) );
    s_captured_len = 0U;
    s_call_count   = 0U;
    log_set_level( LOG_VERBOSE );   /* enable all levels for most tests */
}

void tearDown( void )
{
    /* nothing to tear down */
}

static bool output_contains( const char * substr )
{
    return str_str( s_captured, substr );
}

/* log_set_level */

void test_log_set_level_filters_below( void )
{
    log_set_level( LOG_WARN );
    LOGI( "tag", "should be filtered" );
    TEST_ASSERT_EQUAL_UINT32( 0U, s_call_count );
}

void test_log_set_level_passes_at_level( void )
{
    log_set_level( LOG_WARN );
    LOGW( "tag", "should pass" );
    TEST_ASSERT_EQUAL_UINT32( 1U, s_call_count );
}

void test_log_set_level_passes_above( void )
{
    log_set_level( LOG_WARN );
    LOGE( "tag", "error should pass" );
    TEST_ASSERT_EQUAL_UINT32( 1U, s_call_count );
}

void test_log_set_level_none_filters_all( void )
{
    log_set_level( LOG_NONE );
    LOGE( "tag", "should be filtered" );
    LOGW( "tag", "should be filtered" );
    LOGI( "tag", "should be filtered" );
    TEST_ASSERT_EQUAL_UINT32( 0U, s_call_count );
}

void test_log_set_level_verbose_passes_all( void )
{
    log_set_level( LOG_VERBOSE );
    LOGV( "tag", "verbose" );
    LOGD( "tag", "debug" );
    LOGI( "tag", "info" );
    LOGW( "tag", "warn" );
    LOGE( "tag", "error" );
    TEST_ASSERT_EQUAL_UINT32( 5U, s_call_count );
}

/* Output format */

void test_log_format_contains_level_char( void )
{
    LOGI( "tag", "msg" );
    TEST_ASSERT_TRUE( output_contains( "[I]" ) );
}

void test_log_format_contains_tag( void )
{
    LOGI( "TEST", "msg" );
    TEST_ASSERT_TRUE( output_contains( "(TEST)" ) );
}

void test_log_format_contains_message( void )
{
    LOGI( "tag", "hello world" );
    TEST_ASSERT_TRUE( output_contains( "hello world" ) );
}

void test_log_format_ends_with_crlf( void )
{
    LOGI( "tag", "msg" );
    TEST_ASSERT_TRUE( s_captured_len >= 2U );
    TEST_ASSERT_EQUAL_CHAR( '\r', s_captured[ s_captured_len - 2U ] );
    TEST_ASSERT_EQUAL_CHAR( '\n', s_captured[ s_captured_len - 1U ] );
}

void test_log_format_structure( void )
{
    /* full format: "[I] (tag): msg\r\n" */
    LOGI( "TEST", "session started" );
    TEST_ASSERT_TRUE( output_contains( "[I] (TEST): session started" ) );
}

void test_log_format_null_tag_replaced( void )
{
    log_write( LOG_INFO, NULL, "msg" );
    TEST_ASSERT_TRUE( output_contains( "(?)" ) );
}

void test_log_format_null_msg_safe( void )
{
    /* must not crash */
    log_write( LOG_INFO, "tag", NULL );
    TEST_ASSERT_EQUAL_UINT32( 1U, s_call_count );
}

/* Level strings */

void test_log_level_error_char( void )
{
    LOGE( "t", "m" );
    TEST_ASSERT_TRUE( output_contains( "[E]" ) );
}

void test_log_level_warn_char( void )
{
    LOGW( "t", "m" );
    TEST_ASSERT_TRUE( output_contains( "[W]" ) );
}

void test_log_level_debug_char( void )
{
    LOGD( "t", "m" );
    TEST_ASSERT_TRUE( output_contains( "[D]" ) );
}

void test_log_level_verbose_char( void )
{
    LOGV( "t", "m" );
    TEST_ASSERT_TRUE( output_contains( "[V]" ) );
}

/* Typed write - u32 */

void test_log_write_u32_contains_value( void )
{
    LOGI_U32( "OBD", "speed: ", 96U );
    TEST_ASSERT_TRUE( output_contains( "speed: " ) );
    TEST_ASSERT_TRUE( output_contains( "96" ) );
}

void test_log_write_u32_zero( void )
{
    LOGI_U32( "tag", "val: ", 0U );
    TEST_ASSERT_TRUE( output_contains( "0" ) );
}

void test_log_write_u32_max( void )
{
    LOGI_U32( "tag", "val: ", 4294967295U );
    TEST_ASSERT_TRUE( output_contains( "4294967295" ) );
}

/* Typed write — i32 */

void test_log_write_i32_positive( void )
{
    LOGI_I32( "tag", "val: ", 42 );
    TEST_ASSERT_TRUE( output_contains( "42" ) );
}

void test_log_write_i32_negative( void )
{
    LOGI_I32( "tag", "trim: ", -10 );
    TEST_ASSERT_TRUE( output_contains( "-10" ) );
}

void test_log_write_i32_zero( void )
{
    LOGI_I32( "tag", "val: ", 0 );
    TEST_ASSERT_TRUE( output_contains( "0" ) );
}

/* Typed write — hex */

void test_log_write_hex_value( void )
{
    LOGI_HEX( "UART", "rx: ", 0xDEADBEEFU );
    TEST_ASSERT_TRUE( output_contains( "DEADBEEF" ) );
}

void test_log_write_hex_zero( void )
{
    LOGI_HEX( "tag", "rx: ", 0x00000000U );
    TEST_ASSERT_TRUE( output_contains( "00000000" ) );
}

void test_log_write_hex_byte( void )
{
    LOGI_HEX( "tag", "rx: ", 0xABU );
    TEST_ASSERT_TRUE( output_contains( "000000AB" ) );
}

/* Typed write — f32 */

void test_log_write_f32_positive( void )
{
    LOGI_F32( "OBD", "voltage: ", 14.2F );
    TEST_ASSERT_TRUE( output_contains( "14" ) );
}

void test_log_write_f32_zero( void )
{
    LOGI_F32( "tag", "val: ", 0.0F );
    TEST_ASSERT_TRUE( output_contains( "0" ) );
}

/* Output length */

void test_log_output_len_matches_content( void )
{
    LOGI( "tag", "msg" );
    TEST_ASSERT_EQUAL_UINT32( ( uint32_t )str_len( s_captured ), s_captured_len );
}

void test_log_output_does_not_exceed_max( void )
{
    /* long tag + long message — must not overflow LOG_MAX_MSG_LEN */
    LOGI( "VERYLONGTAG1234", "this is a moderately long message that should still fit" );
    TEST_ASSERT_LESS_OR_EQUAL_UINT32( LOG_MAX_MSG_LEN, s_captured_len );
}

/* Single call per log statement */

void test_log_write_calls_output_once( void )
{
    LOGI( "tag", "msg" );
    TEST_ASSERT_EQUAL_UINT32( 1U, s_call_count );
}

void test_log_write_u32_calls_output_once( void )
{
    LOGI_U32( "tag", "val: ", 42U );
    TEST_ASSERT_EQUAL_UINT32( 1U, s_call_count );
}

int main( void )
{
    UNITY_BEGIN();

    /* level filtering */
    RUN_TEST( test_log_set_level_filters_below );
    RUN_TEST( test_log_set_level_passes_at_level );
    RUN_TEST( test_log_set_level_passes_above );
    RUN_TEST( test_log_set_level_none_filters_all );
    RUN_TEST( test_log_set_level_verbose_passes_all );

    /* format */
    RUN_TEST( test_log_format_contains_level_char );
    RUN_TEST( test_log_format_contains_tag );
    RUN_TEST( test_log_format_contains_message );
    RUN_TEST( test_log_format_ends_with_crlf );
    RUN_TEST( test_log_format_structure );
    RUN_TEST( test_log_format_null_tag_replaced );
    RUN_TEST( test_log_format_null_msg_safe );

    /* level chars */
    RUN_TEST( test_log_level_error_char );
    RUN_TEST( test_log_level_warn_char );
    RUN_TEST( test_log_level_debug_char );
    RUN_TEST( test_log_level_verbose_char );

    /* u32 */
    RUN_TEST( test_log_write_u32_contains_value );
    RUN_TEST( test_log_write_u32_zero );
    RUN_TEST( test_log_write_u32_max );

    /* i32 */
    RUN_TEST( test_log_write_i32_positive );
    RUN_TEST( test_log_write_i32_negative );
    RUN_TEST( test_log_write_i32_zero );

    /* hex */
    RUN_TEST( test_log_write_hex_value );
    RUN_TEST( test_log_write_hex_zero );
    RUN_TEST( test_log_write_hex_byte );

    /* f32 */
    RUN_TEST( test_log_write_f32_positive );
    RUN_TEST( test_log_write_f32_zero );

    /* output integrity */
    RUN_TEST( test_log_output_len_matches_content );
    RUN_TEST( test_log_output_does_not_exceed_max );
    RUN_TEST( test_log_write_calls_output_once );
    RUN_TEST( test_log_write_u32_calls_output_once );

    return UNITY_END();
}
