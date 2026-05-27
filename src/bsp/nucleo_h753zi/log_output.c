/**
 * @file  log_output.c
 * @brief BSP implementation of log_output contract
 */

#include "lib/log/log_output.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/uart.h"

void log_write_output( const char * buf, uint32_t len )
{
    for( uint32_t i = 0U; i < len; ++i )
    {
        ( void )uart_write_byte_blocking( BSP_DEBUG_UART,
                                          ( uint8_t )buf[ i ],
                                          1U );
    }
}
