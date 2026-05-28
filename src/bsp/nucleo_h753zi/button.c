#include "bsp/nucleo_h753zi/button.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/gpio.h"
#include "hal/exti.h"
#include "hal/rcc.h"
#include "hal/nvic.h"

#define BTN_IRQ_PRIORITY  ( 7U )

static btn_t s_user_btn;

void bsp_button_init( void )
{
    rcc_gpio_clk_enable( BSP_BTN_USER_PORT );
    rcc_syscfg_clk_enable();

    gpio_set_mode( BSP_BTN_USER_PORT, BSP_BTN_USER_PIN, GPIO_MODE_INPUT );
    gpio_set_pull( BSP_BTN_USER_PORT, BSP_BTN_USER_PIN, GPIO_PULL_UP );

    exti_set_port_source( EXTI_PORT_PC, BSP_BTN_USER_PIN );
    exti_set_trigger( BSP_BTN_USER_PIN, EXTI_TRIGGER_BOTH );
    exti_unmask( BSP_BTN_USER_PIN );

    nvic_enable_irq( BSP_BTN_IRQn, BTN_IRQ_PRIORITY );
    btn_init( &s_user_btn );
}

bool bsp_button_is_pressed( void )
{
    return btn_is_pressed( &s_user_btn );
}

btn_event_t bsp_button_get_event( void )
{
    return btn_get_event( &s_user_btn );
}

void EXTI15_10_IRQHandler( void )
{
    if( exti_is_pending( BSP_BTN_USER_PIN ) )
    {
        btn_update_raw( &s_user_btn,
                        gpio_read( BSP_BTN_USER_PORT, BSP_BTN_USER_PIN ) );
        exti_clear_pending( BSP_BTN_USER_PIN );
    }
}
