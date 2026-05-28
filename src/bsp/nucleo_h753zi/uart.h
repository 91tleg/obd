#ifndef BSP_NUCLEO_H753ZI_UART_H
#define BSP_NUCLEO_H753ZI_UART_H

#include "lib/core/result.h"
#include "drivers/uart/uart.h"

result_t bsp_uart_debug_init( void );
uart_driver_t * bsp_uart_debug( void );

result_t bsp_uart_obd_init( void );
uart_driver_t * bsp_uart_obd( void );

#endif /* BSP_NUCLEO_H753ZI_UART_H */
