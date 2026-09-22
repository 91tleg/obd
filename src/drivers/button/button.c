#include "drivers/button/button.h"
#include "hal/nvic.h"
#include <stddef.h>

#define BTN_DEBOUNCE_MS  ( 20U )
#define BTN_HOLD_MS      ( 500U )

void btn_init( btn_t * p_btn )
{
    if( p_btn != NULL )
    {
        p_btn->raw            = false;
        p_btn->debounced      = false;
        p_btn->last_debounced = false;
        p_btn->debounce_ms    = 0U;
        p_btn->hold_ms        = 0U;
        p_btn->held_fired     = false;
        p_btn->pending        = BTN_EVENT_NONE;
    }
}

void btn_update_raw( btn_t * p_btn, bool raw )
{
    if( p_btn != NULL )
    {
        /*
         * Called from the EXTI ISR. btn_tick() (the SysTick ISR, higher
         * priority — see button.h) can preempt this between the two writes;
         * without the critical section it could observe a new raw value
         * paired with the old debounce_ms, skipping the debounce restart.
         */
        uint32_t const primask = nvic_enter_critical();

        p_btn->raw         = raw;
        p_btn->debounce_ms = BTN_DEBOUNCE_MS;

        nvic_exit_critical( primask );
    }
}

void btn_tick( btn_t * p_btn )
{
    btn_event_t event = BTN_EVENT_NONE;

    if( p_btn != NULL )
    {
        if( p_btn->debounce_ms > 0U )
        {
            p_btn->debounce_ms--;

            if( p_btn->debounce_ms == 0U )
            {
                p_btn->debounced = !p_btn->raw;  /* active low */

                if( p_btn->debounced && !p_btn->last_debounced )
                {
                    /* new press begins */
                    p_btn->hold_ms    = 0U;
                    p_btn->held_fired = false;
                }
                else if( !p_btn->debounced && p_btn->last_debounced )
                {
                    /* released — a short click only if HELD didn't already fire */
                    if( !p_btn->held_fired )
                    {
                        event = BTN_EVENT_PRESSED;
                    }
                    p_btn->hold_ms = 0U;
                }
                else
                {
                    /* no change after debounce */
                }

                p_btn->last_debounced = p_btn->debounced;
            }
        }

        if( p_btn->debounced && !p_btn->held_fired )
        {
            p_btn->hold_ms++;

            if( p_btn->hold_ms >= BTN_HOLD_MS )
            {
                event              = BTN_EVENT_HELD;
                p_btn->held_fired  = true;
            }
        }

        if( event != BTN_EVENT_NONE )
        {
            p_btn->pending = event;
        }
    }
}

btn_event_t btn_get_event( btn_t * p_btn )
{
    btn_event_t event = BTN_EVENT_NONE;

    if( p_btn != NULL )
    {
        /*
         * Called from the main loop (thread mode — anything can preempt it).
         * Without the critical section, btn_tick() setting a new `pending`
         * between the read and the clear below would be silently wiped out.
         */
        uint32_t const primask = nvic_enter_critical();

        event          = p_btn->pending;
        p_btn->pending = BTN_EVENT_NONE;

        nvic_exit_critical( primask );
    }

    return event;
}

bool btn_is_pressed( btn_t const * p_btn )
{
    bool result = false;

    if( p_btn != NULL )
    {
        result = p_btn->debounced;
    }

    return result;
}
