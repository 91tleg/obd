#include "bsp/nucleo_h753zi/can.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/gpio.h"
#include "hal/rcc.h"
#include "hal/can.h"
#include "hal/nvic.h"

/* More urgent than the debug UART (6). SysTick keeps its reset priority (0), so
 * the CAN ISR can never delay the tick that can_recv() timeouts depend on. */
#define CAN_IRQ_PRIORITY  ( 5U )

_Static_assert( BSP_CAN == FDCAN1, "CAN IRQ wiring below assumes FDCAN1" );

result_t bsp_can_init( void )
{
    result_t result;

    static can_timing_t const s_can_timing = CAN_NBTP( 10U, 1U, 29U, 10U );

    /*
     * No data-phase timing — CAN FD frames will use the nominal bit rate
     * for both arbitration and data phases (BRS disabled).
     *
     * If BRS is needed later, construct with: CAN_DBTP( dsjw, dbrp, dtseg1, dtseg2 )
     */
    static can_data_timing_t const s_data_timing = 0U;

    static can_filter_t const s_can_filter =
    {
        .id_low  = 0x7E8U,   /* CAN_TP_OBD_RESP_BASE */
        .id_high = 0x7EFU,   /* CAN_TP_OBD_RESP_MAX  */
    };

    rcc_gpio_clk_enable( BSP_CAN_TX_PORT );
    rcc_gpio_clk_enable( BSP_CAN_RX_PORT );
    rcc_fdcan_clk_enable();

    gpio_set_mode(  BSP_CAN_TX_PORT, BSP_CAN_TX_PIN, GPIO_MODE_AF         );
    gpio_set_af(    BSP_CAN_TX_PORT, BSP_CAN_TX_PIN, BSP_CAN_TX_AF        );
    gpio_set_speed( BSP_CAN_TX_PORT, BSP_CAN_TX_PIN, GPIO_SPEED_HIGH      );
    gpio_set_pull(  BSP_CAN_TX_PORT, BSP_CAN_TX_PIN, GPIO_PULL_NONE       );
    gpio_set_otype( BSP_CAN_TX_PORT, BSP_CAN_TX_PIN, GPIO_OTYPE_PUSH_PULL );

    gpio_set_mode(  BSP_CAN_RX_PORT, BSP_CAN_RX_PIN, GPIO_MODE_AF         );
    gpio_set_af(    BSP_CAN_RX_PORT, BSP_CAN_RX_PIN, BSP_CAN_RX_AF        );
    gpio_set_speed( BSP_CAN_RX_PORT, BSP_CAN_RX_PIN, GPIO_SPEED_HIGH      );
    gpio_set_pull(  BSP_CAN_RX_PORT, BSP_CAN_RX_PIN, GPIO_PULL_NONE       );
    gpio_set_otype( BSP_CAN_RX_PORT, BSP_CAN_RX_PIN, GPIO_OTYPE_PUSH_PULL );

    result = can_init( BSP_CAN, s_can_timing, s_data_timing, &s_can_filter );

    if( RES_IS_OK( result ) )
    {
        result = can_rx_irq_enable( BSP_CAN );
    }

    if( RES_IS_OK( result ) )
    {
        nvic_enable_irq( FDCAN1_IT0_IRQn, CAN_IRQ_PRIORITY );
    }

    return result;
}

/* Vector table entry (see startup.s): move received frames into the HAL ring. */
void fdcan1_it0_irq_handler( void )
{
    can_irq_handler( BSP_CAN );
}
