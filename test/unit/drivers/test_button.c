/**
 * @file test_button.c
 * @brief Unit tests for the button debounce/hold state machine.
 *
 * Covers the "one action per press" contract: a press yields exactly one
 * of BTN_EVENT_PRESSED (short click, on release) or BTN_EVENT_HELD (long
 * press, fired once while still down) — never both for the same press.
 */

#include "unity.h"
#include <stdint.h>
#include "drivers/button/button.h"

#define BTN_DEBOUNCE_MS  ( 20U )
#define BTN_HOLD_MS      ( 500U )

static btn_t s_btn;

void setUp( void )
{
    btn_init( &s_btn );
}

void tearDown( void )
{
    /* nothing to tear down */
}

/* Tick n times with no raw-level change. */
static void tick_n( uint32_t n )
{
    for( uint32_t i = 0U; i < n; ++i )
    {
        btn_tick( &s_btn );
    }
}

/* Report a debounced level change and let it settle. raw=false = pressed
 * (active low, per btn_tick: debounced = !raw). */
static void press( void )
{
    btn_update_raw( &s_btn, false );
    tick_n( BTN_DEBOUNCE_MS );
}

static void release( void )
{
    btn_update_raw( &s_btn, true );
    tick_n( BTN_DEBOUNCE_MS );
}

void test_init_state_is_idle( void )
{
    TEST_ASSERT_FALSE( btn_is_pressed( &s_btn ) );
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );
}

void test_short_press_emits_pressed_once_on_release( void )
{
    press();
    TEST_ASSERT_TRUE( btn_is_pressed( &s_btn ) );
    /* Nothing pending yet — the click event doesn't fire until release. */
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );

    /* well under the hold threshold */
    tick_n( BTN_HOLD_MS / 4U );

    release();
    TEST_ASSERT_FALSE( btn_is_pressed( &s_btn ) );
    TEST_ASSERT_EQUAL( BTN_EVENT_PRESSED, btn_get_event( &s_btn ) );

    /* consumed — no duplicate */
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );
}

void test_long_press_emits_held_once_and_not_pressed_too( void )
{
    press();

    /* safely under the hold threshold — nothing pending yet */
    tick_n( BTN_HOLD_MS - 100U );
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );

    /* now well past it — HELD must have fired exactly once */
    tick_n( 200U );
    TEST_ASSERT_EQUAL( BTN_EVENT_HELD, btn_get_event( &s_btn ) );

    /* still held for a while longer — HELD must not refire */
    tick_n( 200U );
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );

    /* releasing a long press must NOT also emit a short-click event */
    release();
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );
}

void test_short_press_after_long_press_is_independent( void )
{
    /* long press */
    press();
    tick_n( BTN_HOLD_MS );
    TEST_ASSERT_EQUAL( BTN_EVENT_HELD, btn_get_event( &s_btn ) );
    release();
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );

    /* a fresh short press right after must behave normally */
    press();
    tick_n( BTN_HOLD_MS / 4U );
    release();
    TEST_ASSERT_EQUAL( BTN_EVENT_PRESSED, btn_get_event( &s_btn ) );
}

void test_contact_bounce_during_debounce_does_not_register( void )
{
    /* Several rapid toggles, each restarting the debounce window — the
     * button must not be considered pressed until the level is stable
     * for a full BTN_DEBOUNCE_MS with no further updates. */
    for( uint32_t i = 0U; i < 5U; ++i )
    {
        btn_update_raw( &s_btn, ( i % 2U ) == 0U );
        tick_n( BTN_DEBOUNCE_MS / 2U );
        TEST_ASSERT_FALSE( btn_is_pressed( &s_btn ) );
    }

    /* settle on "released" (raw=true) with no further updates */
    tick_n( BTN_DEBOUNCE_MS );
    TEST_ASSERT_FALSE( btn_is_pressed( &s_btn ) );
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( &s_btn ) );
}

void test_null_safe( void )
{
    btn_init( NULL );
    btn_update_raw( NULL, true );
    btn_tick( NULL );
    TEST_ASSERT_EQUAL( BTN_EVENT_NONE, btn_get_event( NULL ) );
    TEST_ASSERT_FALSE( btn_is_pressed( NULL ) );
}

int main( void )
{
    UNITY_BEGIN();

    RUN_TEST( test_init_state_is_idle );
    RUN_TEST( test_short_press_emits_pressed_once_on_release );
    RUN_TEST( test_long_press_emits_held_once_and_not_pressed_too );
    RUN_TEST( test_short_press_after_long_press_is_independent );
    RUN_TEST( test_contact_bounce_during_debounce_does_not_register );
    RUN_TEST( test_null_safe );

    return UNITY_END();
}
