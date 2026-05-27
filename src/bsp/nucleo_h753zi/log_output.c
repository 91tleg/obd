/**
 * @file  log_output.c
 * @brief BSP implementation of log_output contract
 */

#include "lib/log/log_output.h"
#include "bsp/nucleo_h753zi/uart_debug.h"
#include "drivers/uart/uart.h"

result_t log_write_output( const char * buf, uint32_t len )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( buf != NULL ) && ( len > 0U ) )
    {
        uart_driver_t * drv = bsp_uart_debug();
        uint32_t written = uart_driver_write( drv,
                                              ( uint8_t const * )buf,
                                              len );
        if( written == len )
        {
            result = RES_OK;
        }
        else
        {
            result = RES_ERR_OVERFLOW;
        }
    }

    return result;
}
