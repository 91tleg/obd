#ifndef DRIVERS_BUTTON_BUTTON_CALLBACK_H
#define DRIVERS_BUTTON_BUTTON_CALLBACK_H

#include "drivers/button/button.h"

/**
 * Called by the button driver on press, release, or hold.
 * Define exactly once in application.
 */
void btn_event_callback( btn_event_t event );

#endif /* DRIVERS_BUTTON_BUTTON_CALLBACK_H */
