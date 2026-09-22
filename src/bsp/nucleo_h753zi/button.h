#ifndef BSP_NUCLEO_H753ZI_BUTTON_H
#define BSP_NUCLEO_H753ZI_BUTTON_H

#include "drivers/button/button.h"

void bsp_button_init( void );

/** Advance the debounce/hold state machine. Call from the 1kHz SysTick ISR. */
void bsp_button_tick( void );

bool bsp_button_is_pressed( void );
btn_event_t bsp_button_get_event( void );

#endif /* BSP_NUCLEO_H753ZI_BUTTON_H */
