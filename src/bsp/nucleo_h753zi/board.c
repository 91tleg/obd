#include "bsp/nucleo_h753zi/board.h"
#include "bsp/nucleo_h753zi/uart.h"
#include "bsp/nucleo_h753zi/led.h"
#include "bsp/nucleo_h753zi/i2c.h"
#include "bsp/nucleo_h753zi/button.h"
#include "bsp/nucleo_h753zi/can.h"
#include "bsp/nucleo_h753zi/wdt.h"
#include "hal/nvic.h"

result_t board_init( void )
{
    result_t result;

    nvic_global_disable();

    bsp_wdt_init();  /* watchdog running starting from here */

    result = bsp_i2c_init();

    if( RES_IS_OK( result ) )
    {
        result = bsp_uart_debug_init();
    }

    if( RES_IS_OK( result ) )
    {
        result = bsp_uart_obd_init();
    }

    if( RES_IS_OK( result ) )
    {
        result = bsp_can_init();
    }

    bsp_led_green_init();
    bsp_button_init();

    nvic_global_enable();

    return result;
}
