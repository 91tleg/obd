#include "bsp/nucleo_h753zi/clock.h"
#include "hal/rcc.h"
#include "hal/flash.h"
#include "hal/pwr.h"

uint32_t SystemCoreClock = BSP_SYSCLK_HZ;

void exit_run0_mode( void )
{
    pwr_exit_run_star();
}

void system_init( void )
{
    pwr_set_vos( PWR_VOS1 );

    rcc_hse_enable();
    rcc_prescalers_configure( RCC_D1CFGR_HPRE_DIV2 | RCC_D1CFGR_D1PPRE_DIV2,
                              RCC_D2CFGR_D2PPRE1_DIV2 | RCC_D2CFGR_D2PPRE2_DIV2,
                              RCC_D3CFGR_D3PPRE_DIV2 );
    flash_set_latency( BSP_FLASH_LATENCY );

    rcc_pll1_cfg_t const pll1 =
    {
        .m = BSP_PLL1_M,
        .n = BSP_PLL1_N,
        .p = BSP_PLL1_P,
        .q = BSP_PLL1_Q,
        .r = BSP_PLL1_R,
    };
    rcc_pll1_configure( &pll1 );
    pwr_overdrive_enable();  /* required for VOS1 above 400MHz */
    rcc_pll1_enable();
    rcc_sysclk_switch_pll1();

    SystemCoreClock = bsp_clock_sysclk_hz();
}
