#ifndef DRIVERS_BUTTON_H
#define DRIVERS_BUTTON_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    BTN_EVENT_NONE     = 0U,
    BTN_EVENT_PRESSED  = 1U,
    BTN_EVENT_RELEASED = 2U,
    BTN_EVENT_HELD     = 3U,
} btn_event_t;

typedef struct
{
    bool raw;
    bool debounced;
    bool last_debounced;
    uint32_t debounce_ms;
    uint32_t hold_ms;
    btn_event_t pending;
} btn_t;

void btn_init( btn_t * p_btn );
void btn_tick( btn_t * p_btn );
void btn_update_raw( btn_t * p_btn, bool raw );
btn_event_t btn_get_event( btn_t * p_btn );
bool btn_is_pressed( btn_t const * p_btn );

#endif /* DRIVERS_BUTTON_H */
