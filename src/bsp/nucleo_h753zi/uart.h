#ifndef BSP_NUCLEO_H753ZI_UART_H
#define BSP_NUCLEO_H753ZI_UART_H

#include "drivers/uart/uart.h"

void bsp_uart_debug_init( void );
uart_driver_t * bsp_uart_debug( void );

void bsp_uart_obd_init( void );
uart_driver_t * bsp_uart_obd( void );

#endif /* BSP_NUCLEO_H753ZI_UART_H */
