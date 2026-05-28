#ifndef HAL_PWR_H
#define HAL_PWR_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis/stm32h753xx.h"

typedef enum
{
    PWR_VOS3 = 1U,  /* lowest power — up to 200 MHz                      */
    PWR_VOS2 = 2U,  /* balanced     — up to 300 MHz                      */
    PWR_VOS1 = 3U,  /* performance  — up to 480 MHz (+ overdrive)        */
} pwr_vos_t;

static inline void pwr_exit_run_star( void )
{
    PWR->CR3 |= PWR_CR3_LDOEN;

    while( ( PWR->CSR1 & PWR_CSR1_ACTVOSRDY ) == 0U )
    {
        /* wait for LDO ready */
    }
}

static inline void pwr_set_vos( pwr_vos_t vos )
{
    uint32_t const d3cr = PWR->D3CR;

    PWR->D3CR = ( d3cr & ~PWR_D3CR_VOS_Msk )
                | ( ( uint32_t )vos << PWR_D3CR_VOS_Pos );

    while( ( PWR->CSR1 & PWR_CSR1_ACTVOSRDY ) == 0U )
    {
        /* wait for active voltage scaling ready */
    }
}

static inline pwr_vos_t pwr_get_vos( void )
{
    return ( pwr_vos_t )( ( PWR->D3CR & PWR_D3CR_VOS_Msk )
                            >> PWR_D3CR_VOS_Pos );
}

static inline bool pwr_vos_ready( void )
{
    return ( ( PWR->D3CR & PWR_D3CR_VOSRDY ) != 0U );
}

static inline void pwr_overdrive_enable( void )
{
    SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN;

    while( ( PWR->CSR1 & PWR_CSR1_ACTVOSRDY ) == 0U )
    {
        /* wait for overdrive ready */
    }
}

static inline void pwr_backup_domain_enable( void )
{
    PWR->CR1 |= PWR_CR1_DBP;
}

static inline void pwr_backup_domain_disable( void )
{
    PWR->CR1 &= ~PWR_CR1_DBP;
}

static inline void pwr_enter_sleep( void )
{
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
}

static inline void pwr_enter_stop( void )
{
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
}

#endif /* HAL_PWR_H */
