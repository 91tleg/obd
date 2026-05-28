#ifndef HAL_RCC_H
#define HAL_RCC_H

#include <stdint.h>
#include <stddef.h>
#include "cmsis/stm32h753xx.h"

typedef struct
{
    uint32_t m;
    uint32_t n;
    uint32_t p;
    uint32_t q;
    uint32_t r;
} rcc_pll1_cfg_t;

static inline void rcc_pll1_configure( const rcc_pll1_cfg_t * cfg )
{
    if( cfg != NULL )
    {
        RCC->CR &= ~RCC_CR_PLL1ON;
        while( ( RCC->CR & RCC_CR_PLL1RDY ) != 0U )
        {
            /* wait for PLL1 to stop */
        }

        RCC->PLLCKSELR = RCC_PLLCKSELR_PLLSRC_HSE
                    | ( cfg->m << RCC_PLLCKSELR_DIVM1_Pos );

        RCC->PLLCFGR = RCC_PLLCFGR_PLL1VCOSEL_Msk;         /* wide VCO     */
        RCC->PLLCFGR |= ( 1U << RCC_PLLCFGR_PLL1RGE_Pos ); /* 2-4MHz range */
        RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;               /* P → SYSCLK   */

        RCC->PLL1DIVR = ( ( cfg->n - 1U ) << RCC_PLL1DIVR_N1_Pos )
                      | ( ( cfg->p - 1U ) << RCC_PLL1DIVR_P1_Pos )
                      | ( ( cfg->q - 1U ) << RCC_PLL1DIVR_Q1_Pos )
                      | ( ( cfg->r - 1U ) << RCC_PLL1DIVR_R1_Pos );
    }
}

static inline void rcc_pll1_enable( void )
{
    RCC->CR |= RCC_CR_PLL1ON;
    while( ( RCC->CR & RCC_CR_PLL1RDY ) == 0U )
    {
        /* wait for lock */
    }
}

static inline void rcc_gpio_clk_enable( GPIO_TypeDef * port )
{
    if( port == GPIOA )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOB )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOC )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOD )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOE )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOF )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOG )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOGEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOH )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOHEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOI )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOIEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOJ )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOJEN;
        ( void )RCC->AHB4ENR;
    }
    else if( port == GPIOK )
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_GPIOKEN;
        ( void )RCC->AHB4ENR;
    }
    else
    {
        /* NULL or invalid port — no action */
    }

}

static inline void rcc_gpio_clk_disable( GPIO_TypeDef * port )
{
    if( port == GPIOA ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOAEN; }
    else if( port == GPIOB ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOBEN; }
    else if( port == GPIOC ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOCEN; }
    else if( port == GPIOD ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIODEN; }
    else if( port == GPIOE ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOEEN; }
    else if( port == GPIOF ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOFEN; }
    else if( port == GPIOG ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOGEN; }
    else if( port == GPIOH ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOHEN; }
    else if( port == GPIOI ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOIEN; }
    else if( port == GPIOJ ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOJEN; }
    else if( port == GPIOK ) { RCC->AHB4ENR &= ~RCC_AHB4ENR_GPIOKEN; }
    else { /* NULL or invalid port — no action */ }
}

static inline void rcc_uart_clk_enable( USART_TypeDef * uart )
{
    if( uart == USART2 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_USART2EN;
        ( void )RCC->APB1LENR;
    }
    else if( uart == USART3 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_USART3EN;
        ( void )RCC->APB1LENR;
    }
    else if( uart == UART4 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_UART4EN;
        ( void )RCC->APB1LENR;
    }
    else if( uart == UART5 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_UART5EN;
        ( void )RCC->APB1LENR;
    }
    else if( uart == UART7 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_UART7EN;
        ( void )RCC->APB1LENR;
    }
    else if( uart == UART8 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_UART8EN;
        ( void )RCC->APB1LENR;
    }
    else if( uart == USART1 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
        ( void )RCC->APB2ENR;
    }
    else if( uart == USART6 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
        ( void )RCC->APB2ENR;
    }
    else
    {
        /* NULL or invalid uart — no action */
    }
}

static inline void rcc_uart_clk_disable( USART_TypeDef * uart )
{
    if( uart == USART2 ) { RCC->APB1LENR &= ~RCC_APB1LENR_USART2EN; }
    else if( uart == USART3 ) { RCC->APB1LENR &= ~RCC_APB1LENR_USART3EN; }
    else if( uart == UART4  ) { RCC->APB1LENR &= ~RCC_APB1LENR_UART4EN;  }
    else if( uart == UART5  ) { RCC->APB1LENR &= ~RCC_APB1LENR_UART5EN;  }
    else if( uart == UART7  ) { RCC->APB1LENR &= ~RCC_APB1LENR_UART7EN;  }
    else if( uart == UART8  ) { RCC->APB1LENR &= ~RCC_APB1LENR_UART8EN;  }
    else if( uart == USART1 ) { RCC->APB2ENR  &= ~RCC_APB2ENR_USART1EN;  }
    else if( uart == USART6 ) { RCC->APB2ENR  &= ~RCC_APB2ENR_USART6EN;  }
    else { /* invalid uart — no action */ }
}

static inline void rcc_tim_clk_enable( TIM_TypeDef * tim )
{
    if( tim == TIM2  )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM2EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM3 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM3EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM4 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM4EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM5 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM5EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM6 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM6EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM7 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM7EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM12 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM12EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM13 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM13EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM14 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_TIM14EN;
        ( void )RCC->APB1LENR;
    }
    else if( tim == TIM1 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
        ( void )RCC->APB2ENR;
    }
    else if( tim == TIM8 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
        ( void )RCC->APB2ENR;
    }
    else if( tim == TIM15 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
        ( void )RCC->APB2ENR;
    }
    else if( tim == TIM16 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
        ( void )RCC->APB2ENR;
    }
    else if( tim == TIM17 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
        ( void )RCC->APB2ENR;
    }
    else
    {
        /* invalid timer — no action */
    }
}

static inline void rcc_spi_clk_enable( SPI_TypeDef * spi )
{
    if( spi == SPI1 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        ( void )RCC->APB2ENR;
    }
    else if( spi == SPI2 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_SPI2EN;
        ( void )RCC->APB1LENR;
    }
    else if( spi == SPI3 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_SPI3EN;
        ( void )RCC->APB1LENR;
    }
    else if( spi == SPI4 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;
        ( void )RCC->APB2ENR;
    }
    else if( spi == SPI5 )
    {
        RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;
        ( void )RCC->APB2ENR;
    }
    else if( spi == SPI6 )
    {
        RCC->APB4ENR |= RCC_APB4ENR_SPI6EN;
        ( void )RCC->APB4ENR;
    }
    else
    {
        /* NULL or invalid spi — no action */
    }
}

static inline void rcc_i2c_clk_enable( I2C_TypeDef * i2c )
{
    if( i2c == I2C1 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_I2C1EN;
        ( void )RCC->APB1LENR;
    }
    else if( i2c == I2C2 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_I2C2EN;
        ( void )RCC->APB1LENR;
    }
    else if( i2c == I2C3 )
    {
        RCC->APB1LENR |= RCC_APB1LENR_I2C3EN;
        ( void )RCC->APB1LENR;
    }
    else if( i2c == I2C4 )
    {
        RCC->APB4ENR |= RCC_APB4ENR_I2C4EN;
        ( void )RCC->APB4ENR;
    }
    else
    {
        /* NULL or invalid i2c — no action */
    }
}

static inline void rcc_dma_clk_enable( DMA_TypeDef * dma )
{
    if( dma != NULL )
    {
        if( dma == DMA1 )
        {
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
        }
        else if( dma == DMA2 )
        {
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
        }
        else
        {
            /* invalid dma — no action */
        }

        ( void )RCC->AHB1ENR;
    }
}

static inline void rcc_uart_reset( USART_TypeDef * uart )
{
    if( uart == USART2 )
    {
        RCC->APB1LRSTR |=  RCC_APB1LRSTR_USART2RST;
        RCC->APB1LRSTR &= ~RCC_APB1LRSTR_USART2RST;
    }
    else if( uart == USART3 )
    {
        RCC->APB1LRSTR |=  RCC_APB1LRSTR_USART3RST;
        RCC->APB1LRSTR &= ~RCC_APB1LRSTR_USART3RST;
    }
    else if( uart == UART4 )
    {
        RCC->APB1LRSTR |=  RCC_APB1LRSTR_UART4RST;
        RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART4RST;
    }
    else if( uart == UART5 )
    {
        RCC->APB1LRSTR |=  RCC_APB1LRSTR_UART5RST;
        RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART5RST;
    }
    else if( uart == UART7 )
    {
        RCC->APB1LRSTR |=  RCC_APB1LRSTR_UART7RST;
        RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART7RST;
    }
    else if( uart == UART8 )
    {
        RCC->APB1LRSTR |=  RCC_APB1LRSTR_UART8RST;
        RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART8RST;
    }
    else if( uart == USART1 )
    {
        RCC->APB2RSTR |=  RCC_APB2RSTR_USART1RST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
    }
    else if( uart == USART6 )
    {
        RCC->APB2RSTR |=  RCC_APB2RSTR_USART6RST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_USART6RST;
    }
    else
    {
        /* invalid uart — no action */
    }
}

static inline uint32_t rcc_get_hclk_freq( void )
{
    extern uint32_t SystemCoreClock;
    return SystemCoreClock;
}

static inline uint32_t rcc_get_pclk1_freq( void )
{
    uint32_t const d2ppre1 = ( RCC->D2CFGR & RCC_D2CFGR_D2PPRE1_Msk )
                               >> RCC_D2CFGR_D2PPRE1_Pos;
    uint32_t const shift   = ( ( d2ppre1 & 0x4U ) != 0U )
                               ? ( ( d2ppre1 & 0x3U ) + 1U )
                               : 0U;
    return rcc_get_hclk_freq() >> shift;
}

static inline uint32_t rcc_get_usart28_clk_freq( void )
{
    /* D2CCIP2R USART28SEL = 000 */
    return rcc_get_pclk1_freq();
}

static inline uint32_t rcc_get_pclk2_freq( void )
{
    uint32_t const d2ppre2 = ( RCC->D2CFGR & RCC_D2CFGR_D2PPRE2_Msk )
                               >> RCC_D2CFGR_D2PPRE2_Pos;
    uint32_t const shift   = ( ( d2ppre2 & 0x4U ) != 0U )
                               ? ( ( d2ppre2 & 0x3U ) + 1U )
                               : 0U;
    return rcc_get_hclk_freq() >> shift;
}

static inline void rcc_hse_enable( void )
{
    RCC->CR |= RCC_CR_HSEON;
    while( ( RCC->CR & RCC_CR_HSERDY ) == 0U )
    {
        /* wait */
    }
}

static inline void rcc_prescalers_configure( uint32_t d1cfgr,
                                             uint32_t d2cfgr,
                                             uint32_t d3cfgr )
{
    RCC->D1CFGR = d1cfgr;
    RCC->D2CFGR = d2cfgr;
    RCC->D3CFGR = d3cfgr;
}

static inline void rcc_sysclk_switch_pll1( void )
{
    RCC->CFGR = ( RCC->CFGR & ~RCC_CFGR_SW_Msk ) | RCC_CFGR_SW_PLL1;
    while( ( RCC->CFGR & RCC_CFGR_SWS_Msk ) != RCC_CFGR_SWS_PLL1 )
    {
        /* wait */
    }
}

static inline void rcc_usart_clk_src_hsi( USART_TypeDef * uart )
{
    if( ( uart == USART2 ) || ( uart == USART3 ) ||
        ( uart == UART4  ) || ( uart == UART5  ) ||
        ( uart == UART7  ) || ( uart == UART8  ) )
    {
        RCC->D2CCIP2R &= ~RCC_D2CCIP2R_USART28SEL_Msk;
        RCC->D2CCIP2R |=  RCC_D2CCIP2R_USART28SEL_1;  /* HSI */
    }
    else if( ( uart == USART1 ) || ( uart == USART6 ) )
    {
        RCC->D2CCIP2R &= ~RCC_D2CCIP2R_USART16SEL_Msk;
        RCC->D2CCIP2R |=  RCC_D2CCIP2R_USART16SEL_1;  /* HSI */
    }
    else
    {
        /* NULL or invalid uart — no action */
    }
}

static inline void rcc_syscfg_clk_enable( void )
{
    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;
    ( void )RCC->APB4ENR;
}

static inline void rcc_fdcan_clk_enable( void )
{
    RCC->APB1HENR |= RCC_APB1HENR_FDCANEN;
    ( void )RCC->APB1HENR;
}

static inline void rcc_fdcan_clk_disable( void )
{
    RCC->APB1HENR &= ~RCC_APB1HENR_FDCANEN;
}

#endif /* HAL_RCC_H */
