#include "unity.h"
#include "lib/string/str.h"

void setUp( void )
{
    /* nothing to set up */
}

void tearDown( void )
{
    /* nothing to tear down */
}

/* mem_copy */

void test_mem_copy_basic( void )
{
    uint8_t src[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };
    uint8_t dst[ 4U ] = { 0x00U, 0x00U, 0x00U, 0x00U };

    mem_copy( dst, src, 4U );

    TEST_ASSERT_EQUAL_UINT8( 0x01U, dst[ 0U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x02U, dst[ 1U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x03U, dst[ 2U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x04U, dst[ 3U ] );
}

void test_mem_copy_single_byte( void )
{
    uint8_t src = 0xABU;
    uint8_t dst = 0x00U;

    mem_copy( &dst, &src, 1U );

    TEST_ASSERT_EQUAL_UINT8( 0xABU, dst );
}

void test_mem_copy_zero_len( void )
{
    uint8_t src[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };
    uint8_t dst[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    mem_copy( dst, src, 0U );

    /* dst must be unchanged */
    TEST_ASSERT_EQUAL_UINT8( 0xFFU, dst[ 0U ] );
    TEST_ASSERT_EQUAL_UINT8( 0xFFU, dst[ 1U ] );
}

void test_mem_copy_null_dst( void )
{
    uint8_t src[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };

    /* must not crash */
    mem_copy( NULL, src, 4U );
}

void test_mem_copy_null_src( void )
{
    uint8_t dst[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    /* must not crash, dst unchanged */
    mem_copy( dst, NULL, 4U );

    TEST_ASSERT_EQUAL_UINT8( 0xFFU, dst[ 0U ] );
}

void test_mem_copy_null_both( void )
{
    /* must not crash */
    mem_copy( NULL, NULL, 4U );
}

void test_mem_copy_partial( void )
{
    uint8_t src[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };
    uint8_t dst[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    mem_copy( dst, src, 2U );

    TEST_ASSERT_EQUAL_UINT8( 0x01U, dst[ 0U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x02U, dst[ 1U ] );
    TEST_ASSERT_EQUAL_UINT8( 0xFFU, dst[ 2U ] );  /* untouched */
    TEST_ASSERT_EQUAL_UINT8( 0xFFU, dst[ 3U ] );  /* untouched */
}

/* mem_set */

void test_mem_set_basic( void )
{
    uint8_t buf[ 4U ] = { 0x00U, 0x00U, 0x00U, 0x00U };

    mem_set( buf, 0xAAU, 4U );

    TEST_ASSERT_EQUAL_UINT8( 0xAAU, buf[ 0U ] );
    TEST_ASSERT_EQUAL_UINT8( 0xAAU, buf[ 1U ] );
    TEST_ASSERT_EQUAL_UINT8( 0xAAU, buf[ 2U ] );
    TEST_ASSERT_EQUAL_UINT8( 0xAAU, buf[ 3U ] );
}

void test_mem_set_zero( void )
{
    uint8_t buf[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    mem_set( buf, 0x00U, 4U );

    TEST_ASSERT_EQUAL_UINT8( 0x00U, buf[ 0U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x00U, buf[ 3U ] );
}

void test_mem_set_zero_len( void )
{
    uint8_t buf[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    mem_set( buf, 0x00U, 0U );

    TEST_ASSERT_EQUAL_UINT8( 0xFFU, buf[ 0U ] );  /* untouched */
}

void test_mem_set_null( void )
{
    /* must not crash */
    mem_set( NULL, 0xAAU, 4U );
}

void test_mem_set_single_byte( void )
{
    uint8_t buf[ 4U ] = { 0x00U, 0x00U, 0x00U, 0x00U };

    mem_set( buf, 0xBBU, 1U );

    TEST_ASSERT_EQUAL_UINT8( 0xBBU, buf[ 0U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x00U, buf[ 1U ] );  /* untouched */
}

/* mem_equal */

void test_mem_equal_identical( void )
{
    uint8_t a[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };
    uint8_t b[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };

    TEST_ASSERT_TRUE( mem_equal( a, b, 4U ) );
}

void test_mem_equal_different( void )
{
    uint8_t a[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };
    uint8_t b[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x05U };

    TEST_ASSERT_FALSE( mem_equal( a, b, 4U ) );
}

void test_mem_equal_first_byte_differs( void )
{
    uint8_t a[ 4U ] = { 0xFFU, 0x02U, 0x03U, 0x04U };
    uint8_t b[ 4U ] = { 0x00U, 0x02U, 0x03U, 0x04U };

    TEST_ASSERT_FALSE( mem_equal( a, b, 4U ) );
}

void test_mem_equal_zero_len( void )
{
    uint8_t a[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };
    uint8_t b[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    /* zero length — vacuously equal, nothing to compare */
    TEST_ASSERT_TRUE( mem_equal( a, b, 0U ) );
}

void test_mem_equal_null_a( void )
{
    uint8_t b[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };

    TEST_ASSERT_FALSE( mem_equal( NULL, b, 4U ) );
}

void test_mem_equal_null_b( void )
{
    uint8_t a[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x04U };

    TEST_ASSERT_FALSE( mem_equal( a, NULL, 4U ) );
}

void test_mem_equal_null_both( void )
{
    TEST_ASSERT_FALSE( mem_equal( NULL, NULL, 4U ) );
}

void test_mem_equal_partial( void )
{
    uint8_t a[ 4U ] = { 0x01U, 0x02U, 0x03U, 0xFFU };
    uint8_t b[ 4U ] = { 0x01U, 0x02U, 0x03U, 0x00U };

    /* only compare first 3 bytes — should be equal                       */
    TEST_ASSERT_TRUE( mem_equal( a, b, 3U ) );
}

/* str_len */

void test_str_len_basic( void )
{
    TEST_ASSERT_EQUAL_UINT32( 5U, str_len( "hello" ) );
}

void test_str_len_empty( void )
{
    TEST_ASSERT_EQUAL_UINT32( 0U, str_len( "" ) );
}

void test_str_len_null( void )
{
    TEST_ASSERT_EQUAL_UINT32( 0U, str_len( NULL ) );
}

void test_str_len_single_char( void )
{
    TEST_ASSERT_EQUAL_UINT32( 1U, str_len( "a" ) );
}

void test_str_len_with_spaces( void )
{
    TEST_ASSERT_EQUAL_UINT32( 5U, str_len( "a b c" ) );
}

void test_str_len_numbers( void )
{
    TEST_ASSERT_EQUAL_UINT32( 3U, str_len( "123" ) );
}

/* str_copy */

void test_str_copy_basic( void )
{
    char dst[ 16U ] = { 0 };

    str_copy( dst, "hello", 16U );

    TEST_ASSERT_EQUAL_STRING( "hello", dst );
}

void test_str_copy_exact_fit( void )
{
    char dst[ 6U ] = { 0 };

    str_copy( dst, "hello", 6U );

    TEST_ASSERT_EQUAL_STRING( "hello", dst );
}

void test_str_copy_truncates( void )
{
    char dst[ 4U ] = { 0 };

    str_copy( dst, "hello", 4U );

    /* truncated to 3 chars + null                                         */
    TEST_ASSERT_EQUAL_STRING( "hel", dst );
    TEST_ASSERT_EQUAL_CHAR( '\0', dst[ 3U ] );
}

void test_str_copy_null_dst( void )
{
    /* must not crash */
    str_copy( NULL, "hello", 16U );
}

void test_str_copy_null_src( void )
{
    char dst[ 16U ] = { 0xFFU };

    /* must not crash */
    str_copy( dst, NULL, 16U );
}

void test_str_copy_zero_len( void )
{
    char dst[ 16U ] = { 'X', 'X', 'X', '\0' };

    str_copy( dst, "hello", 0U );

    /* dst unchanged */
    TEST_ASSERT_EQUAL_CHAR( 'X', dst[ 0U ] );
}

void test_str_copy_empty_src( void )
{
    char dst[ 16U ] = { 'X', 0 };

    str_copy( dst, "", 16U );

    TEST_ASSERT_EQUAL_STRING( "", dst );
}

void test_str_copy_always_null_terminated( void )
{
    char dst[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    str_copy( dst, "hello", 4U );

    TEST_ASSERT_EQUAL_CHAR( '\0', dst[ 3U ] );
}

/* str_equal */

void test_str_equal_identical( void )
{
    TEST_ASSERT_TRUE( str_equal( "hello", "hello" ) );
}

void test_str_equal_different( void )
{
    TEST_ASSERT_FALSE( str_equal( "hello", "world" ) );
}

void test_str_equal_empty( void )
{
    TEST_ASSERT_TRUE( str_equal( "", "" ) );
}

void test_str_equal_empty_vs_nonempty( void )
{
    TEST_ASSERT_FALSE( str_equal( "", "hello" ) );
    TEST_ASSERT_FALSE( str_equal( "hello", "" ) );
}

void test_str_equal_null_a( void )
{
    TEST_ASSERT_FALSE( str_equal( NULL, "hello" ) );
}

void test_str_equal_null_b( void )
{
    TEST_ASSERT_FALSE( str_equal( "hello", NULL ) );
}

void test_str_equal_null_both( void )
{
    TEST_ASSERT_FALSE( str_equal( NULL, NULL ) );
}

void test_str_equal_prefix( void )
{
    /* "hell" != "hello" */
    TEST_ASSERT_FALSE( str_equal( "hell", "hello" ) );
}

void test_str_equal_case_sensitive( void )
{
    TEST_ASSERT_FALSE( str_equal( "Hello", "hello" ) );
}

/* str_concat */

void test_str_concat_basic( void )
{
    char dst[ 16U ] = { 0 };

    str_copy( dst, "hello", 16U );
    str_concat( dst, " world", 16U );

    TEST_ASSERT_EQUAL_STRING( "hello world", dst );
}

void test_str_concat_truncates( void )
{
    char dst[ 8U ] = { 0 };

    str_copy( dst, "hello", 8U );
    str_concat( dst, " world", 8U );

    /* "hello w" + null = 8 bytes */
    TEST_ASSERT_EQUAL_STRING( "hello w", dst );
    TEST_ASSERT_EQUAL_CHAR( '\0', dst[ 7U ] );
}

void test_str_concat_null_dst( void )
{
    /* must not crash */
    str_concat( NULL, "world", 16U );
}

void test_str_concat_null_src( void )
{
    char dst[ 16U ] = { 0 };

    str_copy( dst, "hello", 16U );

    /* must not crash, dst unchanged */
    str_concat( dst, NULL, 16U );

    TEST_ASSERT_EQUAL_STRING( "hello", dst );
}

void test_str_concat_zero_len( void )
{
    char dst[ 16U ] = { 0 };

    str_copy( dst, "hello", 16U );
    str_concat( dst, " world", 0U );

    TEST_ASSERT_EQUAL_STRING( "hello", dst );
}

void test_str_concat_empty_src( void )
{
    char dst[ 16U ] = { 0 };

    str_copy( dst, "hello", 16U );
    str_concat( dst, "", 16U );

    TEST_ASSERT_EQUAL_STRING( "hello", dst );
}

void test_str_concat_dst_full( void )
{
    char dst[ 6U ] = { 0 };

    str_copy( dst, "hello", 6U );     /* dst is now full — "hello\0" */
    str_concat( dst, " world", 6U );  /* no room — dst unchanged     */

    TEST_ASSERT_EQUAL_STRING( "hello", dst );
}

/* u32_to_str */

void test_u32_to_str_zero( void )
{
    char buf[ 16U ] = { 0 };

    uint32_t len = u32_to_str( 0U, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "0", buf );
    TEST_ASSERT_EQUAL_UINT32( 1U, len );
}

void test_u32_to_str_basic( void )
{
    char buf[ 16U ] = { 0 };

    uint32_t len = u32_to_str( 12345U, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "12345", buf );
    TEST_ASSERT_EQUAL_UINT32( 5U, len );
}

void test_u32_to_str_max( void )
{
    char buf[ 16U ] = { 0 };

    u32_to_str( 4294967295U, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "4294967295", buf );
}

void test_u32_to_str_truncates( void )
{
    char buf[ 4U ] = { 0 };

    u32_to_str( 12345U, buf, 4U );

    /* truncated to 3 digits + null */
    TEST_ASSERT_EQUAL_CHAR( '\0', buf[ 3U ] );
}

void test_u32_to_str_null_buf( void )
{
    uint32_t len = u32_to_str( 123U, NULL, 16U );

    TEST_ASSERT_EQUAL_UINT32( 0U, len );
}

void test_u32_to_str_zero_buf_len( void )
{
    char buf[ 16U ] = { 'X', 0 };

    uint32_t len = u32_to_str( 123U, buf, 0U );

    TEST_ASSERT_EQUAL_UINT32( 0U, len );
    TEST_ASSERT_EQUAL_CHAR( 'X', buf[ 0U ] );  /* unchanged */
}

void test_u32_to_str_buf_len_one( void )
{
    char buf[ 1U ] = { 'X' };

    uint32_t len = u32_to_str( 123U, buf, 1U );

    TEST_ASSERT_EQUAL_UINT32( 0U, len );
}

void test_u32_to_str_single_digit( void )
{
    char buf[ 16U ] = { 0 };

    u32_to_str( 7U, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "7", buf );
}

/* i32_to_str */

void test_i32_to_str_positive( void )
{
    char buf[ 16U ] = { 0 };

    i32_to_str( 12345, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "12345", buf );
}

void test_i32_to_str_negative( void )
{
    char buf[ 16U ] = { 0 };

    i32_to_str( -12345, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "-12345", buf );
}

void test_i32_to_str_zero( void )
{
    char buf[ 16U ] = { 0 };

    i32_to_str( 0, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "0", buf );
}

void test_i32_to_str_min( void )
{
    char buf[ 16U ] = { 0 };

    i32_to_str( -2147483647, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "-2147483647", buf );
}

void test_i32_to_str_max( void )
{
    char buf[ 16U ] = { 0 };

    i32_to_str( 2147483647, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "2147483647", buf );
}

void test_i32_to_str_null_buf( void )
{
    uint32_t len = i32_to_str( 123, NULL, 16U );

    TEST_ASSERT_EQUAL_UINT32( 0U, len );
}

void test_i32_to_str_negative_one( void )
{
    char buf[ 16U ] = { 0 };

    i32_to_str( -1, buf, 16U );

    TEST_ASSERT_EQUAL_STRING( "-1", buf );
}

/* u8_to_hex */

void test_u8_to_hex_zero( void )
{
    char buf[ 4U ] = { 0 };

    u8_to_hex( 0x00U, buf );

    TEST_ASSERT_EQUAL_STRING( "00", buf );
}

void test_u8_to_hex_ff( void )
{
    char buf[ 4U ] = { 0 };

    u8_to_hex( 0xFFU, buf );

    TEST_ASSERT_EQUAL_STRING( "FF", buf );
}

void test_u8_to_hex_ab( void )
{
    char buf[ 4U ] = { 0 };

    u8_to_hex( 0xABU, buf );

    TEST_ASSERT_EQUAL_STRING( "AB", buf );
}

void test_u8_to_hex_low_nibble( void )
{
    char buf[ 4U ] = { 0 };

    u8_to_hex( 0x0FU, buf );

    TEST_ASSERT_EQUAL_STRING( "0F", buf );
}

void test_u8_to_hex_high_nibble( void )
{
    char buf[ 4U ] = { 0 };

    u8_to_hex( 0xF0U, buf );

    TEST_ASSERT_EQUAL_STRING( "F0", buf );
}

void test_u8_to_hex_null( void )
{
    uint32_t result = u8_to_hex( 0xABU, NULL );

    TEST_ASSERT_EQUAL_UINT32( 0U, result );
}

void test_u8_to_hex_null_terminated( void )
{
    char buf[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    u8_to_hex( 0xABU, buf );

    TEST_ASSERT_EQUAL_CHAR( '\0', buf[ 2U ] );
}

void test_u8_to_hex_returns_two( void )
{
    char     buf[ 4U ] = { 0 };
    uint32_t result    = u8_to_hex( 0xABU, buf );

    TEST_ASSERT_EQUAL_UINT32( 2U, result );
}

/* mem_zero (uses mem_set internally) */

void test_mem_zero_basic( void )
{
    uint8_t buf[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    mem_zero( buf, 4U );

    TEST_ASSERT_EQUAL_UINT8( 0x00U, buf[ 0U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x00U, buf[ 1U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x00U, buf[ 2U ] );
    TEST_ASSERT_EQUAL_UINT8( 0x00U, buf[ 3U ] );
}

void test_mem_zero_null( void )
{
    /* must not crash */
    mem_zero( NULL, 4U );
}

void test_mem_zero_zero_len( void )
{
    uint8_t buf[ 4U ] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

    mem_zero( buf, 0U );

    TEST_ASSERT_EQUAL_UINT8( 0xFFU, buf[ 0U ] );
}

int main( void )
{
    UNITY_BEGIN();

    /* mem_copy */
    RUN_TEST( test_mem_copy_basic );
    RUN_TEST( test_mem_copy_single_byte );
    RUN_TEST( test_mem_copy_zero_len );
    RUN_TEST( test_mem_copy_null_dst );
    RUN_TEST( test_mem_copy_null_src );
    RUN_TEST( test_mem_copy_null_both );
    RUN_TEST( test_mem_copy_partial );

    /* mem_set */
    RUN_TEST( test_mem_set_basic );
    RUN_TEST( test_mem_set_zero );
    RUN_TEST( test_mem_set_zero_len );
    RUN_TEST( test_mem_set_null );
    RUN_TEST( test_mem_set_single_byte );

    /* mem_equal */
    RUN_TEST( test_mem_equal_identical );
    RUN_TEST( test_mem_equal_different );
    RUN_TEST( test_mem_equal_first_byte_differs );
    RUN_TEST( test_mem_equal_zero_len );
    RUN_TEST( test_mem_equal_null_a );
    RUN_TEST( test_mem_equal_null_b );
    RUN_TEST( test_mem_equal_null_both );
    RUN_TEST( test_mem_equal_partial );

    /* str_len */
    RUN_TEST( test_str_len_basic );
    RUN_TEST( test_str_len_empty );
    RUN_TEST( test_str_len_null );
    RUN_TEST( test_str_len_single_char );
    RUN_TEST( test_str_len_with_spaces );
    RUN_TEST( test_str_len_numbers );

    /* str_copy */
    RUN_TEST( test_str_copy_basic );
    RUN_TEST( test_str_copy_exact_fit );
    RUN_TEST( test_str_copy_truncates );
    RUN_TEST( test_str_copy_null_dst );
    RUN_TEST( test_str_copy_null_src );
    RUN_TEST( test_str_copy_zero_len );
    RUN_TEST( test_str_copy_empty_src );
    RUN_TEST( test_str_copy_always_null_terminated );

    /* str_equal */
    RUN_TEST( test_str_equal_identical );
    RUN_TEST( test_str_equal_different );
    RUN_TEST( test_str_equal_empty );
    RUN_TEST( test_str_equal_empty_vs_nonempty );
    RUN_TEST( test_str_equal_null_a );
    RUN_TEST( test_str_equal_null_b );
    RUN_TEST( test_str_equal_null_both );
    RUN_TEST( test_str_equal_prefix );
    RUN_TEST( test_str_equal_case_sensitive );

    /* str_concat */
    RUN_TEST( test_str_concat_basic );
    RUN_TEST( test_str_concat_truncates );
    RUN_TEST( test_str_concat_null_dst );
    RUN_TEST( test_str_concat_null_src );
    RUN_TEST( test_str_concat_zero_len );
    RUN_TEST( test_str_concat_empty_src );
    RUN_TEST( test_str_concat_dst_full );

    /* u32_to_str */
    RUN_TEST( test_u32_to_str_zero );
    RUN_TEST( test_u32_to_str_basic );
    RUN_TEST( test_u32_to_str_max );
    RUN_TEST( test_u32_to_str_truncates );
    RUN_TEST( test_u32_to_str_null_buf );
    RUN_TEST( test_u32_to_str_zero_buf_len );
    RUN_TEST( test_u32_to_str_buf_len_one );
    RUN_TEST( test_u32_to_str_single_digit );

    /* i32_to_str */
    RUN_TEST( test_i32_to_str_positive );
    RUN_TEST( test_i32_to_str_negative );
    RUN_TEST( test_i32_to_str_zero );
    RUN_TEST( test_i32_to_str_min );
    RUN_TEST( test_i32_to_str_max );
    RUN_TEST( test_i32_to_str_null_buf );
    RUN_TEST( test_i32_to_str_negative_one );

    /* u8_to_hex */
    RUN_TEST( test_u8_to_hex_zero );
    RUN_TEST( test_u8_to_hex_ff );
    RUN_TEST( test_u8_to_hex_ab );
    RUN_TEST( test_u8_to_hex_low_nibble );
    RUN_TEST( test_u8_to_hex_high_nibble );
    RUN_TEST( test_u8_to_hex_null );
    RUN_TEST( test_u8_to_hex_null_terminated );
    RUN_TEST( test_u8_to_hex_returns_two );

    /* mem_zero */
    RUN_TEST( test_mem_zero_basic );
    RUN_TEST( test_mem_zero_null );
    RUN_TEST( test_mem_zero_zero_len );

    return UNITY_END();
}
