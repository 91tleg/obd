#include "unity.h"
#include "lib/math/ema.h"

void setUp( void )
{
    /* nothing to set up */
}

void tearDown( void )
{
    /* nothing to tear down */
}

/* Integer EMA */

void test_ema_init_sets_value( void )
{
    ema_t ema;
    ema_init( &ema, EMA_ALPHA_0_5, 100U );
    TEST_ASSERT_EQUAL_UINT32( 100U, ema_get( &ema ) );
}

void test_ema_init_sets_alpha( void )
{
    ema_t ema;
    ema_init( &ema, EMA_ALPHA_0_5, 0U );
    TEST_ASSERT_EQUAL_UINT8( EMA_ALPHA_0_5, ema.alpha );
}

void test_ema_init_null_safe( void )
{
    /* must not crash */
    ema_init( NULL, EMA_ALPHA_0_5, 100U );
}

void test_ema_get_null_returns_zero( void )
{
    TEST_ASSERT_EQUAL_UINT32( 0U, ema_get( NULL ) );
}

void test_ema_update_null_returns_zero( void )
{
    TEST_ASSERT_EQUAL_UINT32( 0U, ema_update( NULL, 100U ) );
}

void test_ema_update_alpha_1_tracks_input( void )
{
    /* alpha = 255 ≈ 1.0 — output should closely track input */
    ema_t ema;
    uint32_t result;

    ema_init( &ema, 255U, 0U );
    result = ema_update( &ema, 200U );

    /* with alpha=255/256 ≈ 1, result should be very close to 200 */
    TEST_ASSERT_UINT32_WITHIN( 2U, 200U, result );
}

void test_ema_update_alpha_0_holds_value( void )
{
    /* alpha = 1 (minimum) — output barely moves from initial */
    ema_t ema;
    uint32_t result;

    ema_init( &ema, 1U, 50U );
    result = ema_update( &ema, 200U );

    /* with alpha=1/256, output should stay very close to 50 */
    TEST_ASSERT_UINT32_WITHIN( 2U, 50U, result );
}

void test_ema_update_alpha_half_converges( void )
{
    /* alpha = 128 = 0.5 — after one step from 0 toward 100:
     * expected = 0.5 * 100 + 0.5 * 0 = 50 */
    ema_t ema;
    uint32_t result;

    ema_init( &ema, EMA_ALPHA_0_5, 0U );
    result = ema_update( &ema, 100U );

    /* allow ±1 for fixed-point rounding */
    TEST_ASSERT_UINT32_WITHIN( 1U, 50U, result );
}

void test_ema_update_steady_state( void )
{
    /* feeding constant value — output must converge to that value */
    ema_t ema;
    uint32_t result = 0U;

    ema_init( &ema, EMA_ALPHA_0_5, 0U );

    for( uint32_t i = 0U; i < 32U; ++i )
    {
        result = ema_update( &ema, 100U );
    }

    /* after many steps, output must be within 1 of 100 */
    TEST_ASSERT_UINT32_WITHIN( 1U, 100U, result );
}

void test_ema_update_step_response( void )
{
    /* step from 0 to 256 — check output is between old and new value
     * after one update with alpha=0.5 */
    ema_t ema;
    uint32_t result;

    ema_init( &ema, EMA_ALPHA_0_5, 0U );
    result = ema_update( &ema, 256U );

    TEST_ASSERT_GREATER_THAN_UINT32( 0U, result );
    TEST_ASSERT_LESS_THAN_UINT32( 256U, result );
}

void test_ema_input_max_no_overflow( void )
{
    /* EMA_INPUT_MAX is the documented safe upper bound */
    ema_t ema;
    uint32_t result;

    ema_init( &ema, EMA_ALPHA_0_5, EMA_INPUT_MAX );
    result = ema_update( &ema, EMA_INPUT_MAX );

    TEST_ASSERT_EQUAL_UINT32( EMA_INPUT_MAX, result );
}

void test_ema_get_returns_current_without_update( void )
{
    ema_t ema;

    ema_init( &ema, EMA_ALPHA_0_5, 42U );
    TEST_ASSERT_EQUAL_UINT32( 42U, ema_get( &ema ) );

    /* value must not change without an update call */
    TEST_ASSERT_EQUAL_UINT32( 42U, ema_get( &ema ) );
}

/* Float EMA */

void test_ema_f32_init_sets_value( void )
{
    ema_f32_t ema;
    ema_f32_init( &ema, 0.5F, 10.0F );
    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 10.0F, ema_f32_get( &ema ) );
}

void test_ema_f32_init_null_safe( void )
{
    ema_f32_init( NULL, 0.5F, 10.0F );
}

void test_ema_f32_get_null_returns_zero( void )
{
    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 0.0F, ema_f32_get( NULL ) );
}

void test_ema_f32_update_null_returns_zero( void )
{
    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 0.0F, ema_f32_update( NULL, 1.0F ) );
}

void test_ema_f32_update_alpha_half_one_step( void )
{
    /* alpha=0.5, initial=0, sample=100 → expected = 0 + 0.5*(100-0) = 50 */
    ema_f32_t ema;
    float result;

    ema_f32_init( &ema, 0.5F, 0.0F );
    result = ema_f32_update( &ema, 100.0F );

    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 50.0F, result );
}

void test_ema_f32_update_alpha_1_tracks_input( void )
{
    /* alpha=1.0 — output equals input immediately */
    ema_f32_t ema;
    float result;

    ema_f32_init( &ema, 1.0F, 0.0F );
    result = ema_f32_update( &ema, 75.0F );

    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 75.0F, result );
}

void test_ema_f32_update_alpha_0_holds_value( void )
{
    /* alpha=0.0 — output never changes */
    ema_f32_t ema;
    float result;

    ema_f32_init( &ema, 0.0F, 33.0F );
    result = ema_f32_update( &ema, 999.0F );

    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 33.0F, result );
}

void test_ema_f32_update_steady_state( void )
{
    /* feeding constant value — must converge to that value */
    ema_f32_t ema;
    float result = 0.0F;

    ema_f32_init( &ema, 0.1F, 0.0F );

    for( uint32_t i = 0U; i < 100U; i++ )
    {
        result = ema_f32_update( &ema, 50.0F );
    }

    TEST_ASSERT_FLOAT_WITHIN( 0.01F, 50.0F, result );
}

void test_ema_f32_update_step_response( void )
{
    /* step from 0 to 100 — output must be between old and new */
    ema_f32_t ema;
    float result;

    ema_f32_init( &ema, 0.5F, 0.0F );
    result = ema_f32_update( &ema, 100.0F );

    TEST_ASSERT_GREATER_THAN( 0.0F,   result );
    TEST_ASSERT_LESS_THAN( 100.0F, result );
}

void test_ema_f32_get_returns_current_without_update( void )
{
    ema_f32_t ema;

    ema_f32_init( &ema, 0.5F, 7.5F );
    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 7.5F, ema_f32_get( &ema ) );
    TEST_ASSERT_FLOAT_WITHIN( 0.001F, 7.5F, ema_f32_get( &ema ) );
}

void test_ema_f32_negative_values( void )
{
    ema_f32_t ema;
    float result;

    ema_f32_init( &ema, 0.5F, 0.0F );
    result = ema_f32_update( &ema, -100.0F );

    TEST_ASSERT_FLOAT_WITHIN( 0.001F, -50.0F, result );
}

int main( void )
{
    UNITY_BEGIN();

    /* integer */
    RUN_TEST( test_ema_init_sets_value );
    RUN_TEST( test_ema_init_sets_alpha );
    RUN_TEST( test_ema_init_null_safe );
    RUN_TEST( test_ema_get_null_returns_zero );
    RUN_TEST( test_ema_update_null_returns_zero );
    RUN_TEST( test_ema_update_alpha_1_tracks_input );
    RUN_TEST( test_ema_update_alpha_0_holds_value );
    RUN_TEST( test_ema_update_alpha_half_converges );
    RUN_TEST( test_ema_update_steady_state );
    RUN_TEST( test_ema_update_step_response );
    RUN_TEST( test_ema_input_max_no_overflow );
    RUN_TEST( test_ema_get_returns_current_without_update );

    /* float */
    RUN_TEST( test_ema_f32_init_sets_value );
    RUN_TEST( test_ema_f32_init_null_safe );
    RUN_TEST( test_ema_f32_get_null_returns_zero );
    RUN_TEST( test_ema_f32_update_null_returns_zero );
    RUN_TEST( test_ema_f32_update_alpha_half_one_step );
    RUN_TEST( test_ema_f32_update_alpha_1_tracks_input );
    RUN_TEST( test_ema_f32_update_alpha_0_holds_value );
    RUN_TEST( test_ema_f32_update_steady_state );
    RUN_TEST( test_ema_f32_update_step_response );
    RUN_TEST( test_ema_f32_get_returns_current_without_update );
    RUN_TEST( test_ema_f32_negative_values );

    return UNITY_END();
}
