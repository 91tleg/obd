#include "drivers/button/button_callback.h"

void btn_event_callback( btn_event_t event )
{
    switch( event )
    {
        case BTN_EVENT_PRESSED:
            break;
        case BTN_EVENT_RELEASED:
            break;
        case BTN_EVENT_HELD:
            break;
        case BTN_EVENT_NONE:
            break;
        default:
            break;
    }
}
