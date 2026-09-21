#include "bsp/nucleo_h753zi/board.h"
#include "bsp/nucleo_h753zi/uart.h"
#include "bsp/nucleo_h753zi/led.h"
#include "bsp/nucleo_h753zi/i2c.h"
#include "bsp/nucleo_h753zi/button.h"
#include "bsp/nucleo_h753zi/can.h"
#include "bsp/nucleo_h753zi/wdt.h"
#include "bsp/nucleo_h753zi/systick.h"
#include "hal/nvic.h"

result_t board_init( void )
{
    result_t result;

    nvic_global_disable();

    bsp_systick_init();

    bsp_wdt_init();  /* watchdog running starting from here */

    result = bsp_i2c_init();

    if( RES_IS_OK( result ) )
    {
        result = bsp_uart_debug_init();
    }

#if defined( BSP_OBD_KWP2000 )
    if( RES_IS_OK( result ) )
    {
        result = bsp_uart_obd_init();
    }
#elif defined( BSP_OBD_CAN )
    if( RES_IS_OK( result ) )
    {
        result = bsp_can_init();
    }
#else
#error "No OBD transport selected: define BSP_OBD_CAN or BSP_OBD_KWP2000"
#endif

    bsp_led_green_init();
    bsp_button_init();

    nvic_global_enable();

    return result;
}
