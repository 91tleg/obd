#include "button.h"

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
        p_btn->pending        = BTN_EVENT_NONE;
    }
}

void btn_update_raw( btn_t * p_btn, bool raw )
{
    if( p_btn != NULL )
    {
        p_btn->raw         = raw;
        p_btn->debounce_ms = BTN_DEBOUNCE_MS;
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
                    p_btn->hold_ms = 0U;
                    event = BTN_EVENT_PRESSED;
                }
                else if( !p_btn->debounced && p_btn->last_debounced )
                {
                    p_btn->hold_ms = 0U;
                    event = BTN_EVENT_RELEASED;
                }
                else
                {
                    /* no change after debounce */
                }

                p_btn->last_debounced = p_btn->debounced;
            }
        }

        if( p_btn->debounced )
        {
            p_btn->hold_ms++;

            if( p_btn->hold_ms == BTN_HOLD_MS )
            {
                event = BTN_EVENT_HELD;
            }
        }

        if( event != BTN_EVENT_NONE )
        {
            p_btn->pending = event;
            btn_event_callback( event );
        }
    }
}

btn_event_t btn_get_event( btn_t * p_btn )
{
    btn_event_t event = BTN_EVENT_NONE;

    if( p_btn != NULL )
    {
        event = p_btn->pending;
        p_btn->pending = BTN_EVENT_NONE;
    }

    return event;
}

bool btn_is_pressed( const btn_t * p_btn )
{
    bool result = false;

    if( p_btn != NULL )
    {
        result = p_btn->debounced;
    }

    return result;
}
