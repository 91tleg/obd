#ifndef DRIVERS_BUTTON_H
#define DRIVERS_BUTTON_H

#include <stdint.h>
#include <stdbool.h>

/*
 * A press yields exactly one event, never both:
 *   - held past BTN_HOLD_MS while still down  -> BTN_EVENT_HELD (fires once,
 *     at the moment the threshold is crossed, while still pressed)
 *   - released before BTN_HOLD_MS             -> BTN_EVENT_PRESSED (fires
 *     once, on release — this is the "short click")
 * BTN_EVENT_RELEASED is not emitted; nothing currently needs a distinct
 * "released after a long press" notification.
 */
typedef enum
{
    BTN_EVENT_NONE     = 0U,
    BTN_EVENT_PRESSED  = 1U,   /* short press, fires on release           */
    BTN_EVENT_RELEASED = 2U,   /* reserved — not currently emitted        */
    BTN_EVENT_HELD     = 3U,   /* long press, fires once while still held */
} btn_event_t;

/*
 * Written by btn_update_raw() (ISR context) and btn_tick() (whichever
 * context drives it, expected to be a fixed-rate tick ISR — see btn_tick()).
 * volatile because both writers can run in interrupt context and race with
 * readers in ordinary code; callers still need a critical section around any
 * multi-field read-modify-write (btn_update_raw() and btn_get_event() already
 * do this) since volatile alone does not make a sequence atomic.
 */
typedef struct
{
    volatile bool         raw;
    volatile bool         debounced;
    volatile bool         last_debounced;
    volatile uint32_t     debounce_ms;
    volatile uint32_t     hold_ms;
    volatile bool         held_fired;   /* HELD already emitted this press */
    volatile btn_event_t  pending;
} btn_t;

void btn_init( btn_t * p_btn );

/**
 * Advance the debounce/hold state machine by one call. Must be called at a
 * fixed 1ms rate (BTN_DEBOUNCE_MS/BTN_HOLD_MS are millisecond counts) — the
 * intended driver is a 1kHz tick ISR, not the main loop, whose iteration
 * time is not constant.
 */
void btn_tick( btn_t * p_btn );

/** Report the current raw (undebounced) pin state. Call from the ISR. */
void btn_update_raw( btn_t * p_btn, bool raw );

/** Consume and clear the pending event, if any. Safe to call from any context. */
btn_event_t btn_get_event( btn_t * p_btn );

bool btn_is_pressed( btn_t const * p_btn );

#endif /* DRIVERS_BUTTON_H */
