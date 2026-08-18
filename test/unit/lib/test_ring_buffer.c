#include "unity.h"
#include <stdbool.h>
#include <stdint.h>
#include "lib/collections/ring_buffer/ring_buffer.h"

#define TEST_BUF_SIZE  ( 8U )

static ring_buf_t s_rb;
static uint8_t    s_buf[ TEST_BUF_SIZE ];

void setUp( void )
{
    ring_buf_init( &s_rb, s_buf, TEST_BUF_SIZE );
}

void tearDown( void )
{
    /* nothing to tear down */
}

void test_ring_buf_init_valid( void )
{
    TEST_ASSERT_EQUAL_PTR( s_buf, s_rb.buf );
    TEST_ASSERT_EQUAL_UINT32( TEST_BUF_SIZE, s_rb.size );
    TEST_ASSERT_EQUAL_UINT32( 0U, s_rb.head );
    TEST_ASSERT_EQUAL_UINT32( 0U, s_rb.tail );
}

void test_ring_buf_init_null_buffer( void )
{
    ring_buf_t rb = { 0 };

    ring_buf_init( &rb, NULL, TEST_BUF_SIZE );

    TEST_ASSERT_NULL( rb.buf );
    TEST_ASSERT_EQUAL_UINT32( 0U, rb.size );
}

void test_ring_buf_init_null_ring_buffer( void )
{
    ring_buf_init( NULL, s_buf, TEST_BUF_SIZE );
}

void test_ring_buf_init_non_power_of_two_size( void )
{
    ring_buf_t rb = { 0 };
    uint8_t    buf[ 7U ];

    ring_buf_init( &rb, buf, 7U );

    TEST_ASSERT_NULL( rb.buf );
    TEST_ASSERT_EQUAL_UINT32( 0U, rb.size );
}

void test_ring_buf_push_single_byte( void )
{
    bool result = ring_buf_push( &s_rb, 0x55U );

    TEST_ASSERT_TRUE( result );
    TEST_ASSERT_EQUAL_UINT32( 1U, ring_buf_used( &s_rb ) );
}

void test_ring_buf_pop_single_byte( void )
{
    uint8_t data = 0U;

    ( void )ring_buf_push( &s_rb, 0xAAU );

    TEST_ASSERT_TRUE( ring_buf_pop( &s_rb, &data ) );
    TEST_ASSERT_EQUAL_HEX8( 0xAAU, data );
    TEST_ASSERT_TRUE( ring_buf_empty( &s_rb ) );
}

void test_ring_buf_peek_does_not_remove( void )
{
    uint8_t data = 0U;

    ( void )ring_buf_push( &s_rb, 0x42U );

    TEST_ASSERT_TRUE( ring_buf_peek( &s_rb, &data ) );
    TEST_ASSERT_EQUAL_HEX8( 0x42U, data );
    TEST_ASSERT_EQUAL_UINT32( 1U, ring_buf_used( &s_rb ) );
}

void test_ring_buf_push_until_full( void )
{
    uint32_t i = 0U;

    for( i = 0U; i < TEST_BUF_SIZE; i++ )
    {
        TEST_ASSERT_TRUE( ring_buf_push( &s_rb, (uint8_t)i ) );
    }

    TEST_ASSERT_TRUE( ring_buf_full( &s_rb ) );
    TEST_ASSERT_EQUAL_UINT32( 0U, ring_buf_free( &s_rb ) );
}

void test_ring_buf_push_when_full( void )
{
    uint32_t i = 0U;

    for( i = 0U; i < TEST_BUF_SIZE; i++ )
    {
        ( void )ring_buf_push( &s_rb, (uint8_t)i );
    }

    TEST_ASSERT_FALSE( ring_buf_push( &s_rb, 0xFFU ) );
}

void test_ring_buf_pop_when_empty( void )
{
    uint8_t data = 0U;

    TEST_ASSERT_FALSE( ring_buf_pop( &s_rb, &data ) );
}

void test_ring_buf_peek_when_empty( void )
{
    uint8_t data = 0U;

    TEST_ASSERT_FALSE( ring_buf_peek( &s_rb, &data ) );
}

void test_ring_buf_fifo_order( void )
{
    uint8_t data = 0U;

    ( void )ring_buf_push( &s_rb, 0x11U );
    ( void )ring_buf_push( &s_rb, 0x22U );
    ( void )ring_buf_push( &s_rb, 0x33U );

    ( void )ring_buf_pop( &s_rb, &data );
    TEST_ASSERT_EQUAL_HEX8( 0x11U, data );

    ( void )ring_buf_pop( &s_rb, &data );
    TEST_ASSERT_EQUAL_HEX8( 0x22U, data );

    ( void )ring_buf_pop( &s_rb, &data );
    TEST_ASSERT_EQUAL_HEX8( 0x33U, data );
}

void test_ring_buf_wraparound( void )
{
    uint8_t data = 0U;
    uint32_t i = 0U;

    for( ; i < TEST_BUF_SIZE; ++i )
    {
        ( void )ring_buf_push( &s_rb, (uint8_t)i );
    }

    for( i = 0U; i < 4U; i++ )
    {
        ( void )ring_buf_pop( &s_rb, &data );
    }

    for( i = 0U; i < 4U; i++ )
    {
        TEST_ASSERT_TRUE( ring_buf_push( &s_rb, (uint8_t)( i + 10U ) ) );
    }

    TEST_ASSERT_TRUE( ring_buf_full( &s_rb ) );
}

void test_ring_buf_used_and_free( void )
{
    ( void )ring_buf_push( &s_rb, 0x01U );
    ( void )ring_buf_push( &s_rb, 0x02U );

    TEST_ASSERT_EQUAL_UINT32( 2U, ring_buf_used( &s_rb ) );
    TEST_ASSERT_EQUAL_UINT32( 6U, ring_buf_free( &s_rb ) );
}

void test_ring_buf_clear( void )
{
    ( void )ring_buf_push( &s_rb, 0x01U );
    ( void )ring_buf_push( &s_rb, 0x02U );

    ring_buf_clear( &s_rb );

    TEST_ASSERT_TRUE( ring_buf_empty( &s_rb ) );
    TEST_ASSERT_EQUAL_UINT32( TEST_BUF_SIZE, ring_buf_free( &s_rb ) );
}

void test_ring_buf_null_push( void )
{
    TEST_ASSERT_FALSE( ring_buf_push( NULL, 0x11U ) );
}

void test_ring_buf_null_pop_buffer( void )
{
    TEST_ASSERT_FALSE( ring_buf_pop( NULL, s_buf ) );
}

void test_ring_buf_null_pop_data( void )
{
    TEST_ASSERT_FALSE( ring_buf_pop( &s_rb, NULL ) );
}

void test_ring_buf_null_peek_buffer( void )
{
    TEST_ASSERT_FALSE( ring_buf_peek( NULL, s_buf ) );
}

void test_ring_buf_null_peek_data( void )
{
    TEST_ASSERT_FALSE( ring_buf_peek( &s_rb, NULL ) );
}

void test_ring_buf_null_used( void )
{
    TEST_ASSERT_EQUAL_UINT32( 0U, ring_buf_used( NULL ) );
}

void test_ring_buf_null_free( void )
{
    TEST_ASSERT_EQUAL_UINT32( 0U, ring_buf_free( NULL ) );
}

void test_ring_buf_null_full( void )
{
    TEST_ASSERT_FALSE( ring_buf_full( NULL ) );
}

void test_ring_buf_null_empty( void )
{
    TEST_ASSERT_TRUE( ring_buf_empty( NULL ) );
}

void test_ring_buf_clear_null( void )
{
    ring_buf_clear( NULL );
}

int main( void )
{
    UNITY_BEGIN();

    RUN_TEST( test_ring_buf_init_valid );
    RUN_TEST( test_ring_buf_init_null_buffer );
    RUN_TEST( test_ring_buf_init_null_ring_buffer );
    RUN_TEST( test_ring_buf_init_non_power_of_two_size );
    RUN_TEST( test_ring_buf_push_single_byte );
    RUN_TEST( test_ring_buf_pop_single_byte );
    RUN_TEST( test_ring_buf_peek_does_not_remove );
    RUN_TEST( test_ring_buf_push_until_full );
    RUN_TEST( test_ring_buf_push_when_full );
    RUN_TEST( test_ring_buf_pop_when_empty );
    RUN_TEST( test_ring_buf_peek_when_empty );
    RUN_TEST( test_ring_buf_fifo_order );
    RUN_TEST( test_ring_buf_wraparound );
    RUN_TEST( test_ring_buf_used_and_free );
    RUN_TEST( test_ring_buf_clear );
    RUN_TEST( test_ring_buf_null_push );
    RUN_TEST( test_ring_buf_null_pop_buffer );
    RUN_TEST( test_ring_buf_null_pop_data );
    RUN_TEST( test_ring_buf_null_peek_buffer );
    RUN_TEST( test_ring_buf_null_peek_data );
    RUN_TEST( test_ring_buf_null_used );
    RUN_TEST( test_ring_buf_null_free );
    RUN_TEST( test_ring_buf_null_full );
    RUN_TEST( test_ring_buf_null_empty );
    RUN_TEST( test_ring_buf_clear_null );

    return UNITY_END();
}
