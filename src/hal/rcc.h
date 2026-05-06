#ifndef HAL_RCC_H
#define HAL_RCC_H

#include <stdint.h>
#include "stm32h753xx.h"

static inline void rcc_gpio_clk_enable( GPIO_TypeDef * port )
{
    if( port == GPIOA ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN; }
    else if( port == GPIOB ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN; }
    else if( port == GPIOC ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN; }
    else if( port == GPIOD ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN; }
    else if( port == GPIOE ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN; }
    else if( port == GPIOF ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN; }
    else if( port == GPIOG ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOGEN; }
    else if( port == GPIOH ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOHEN; }
    else if( port == GPIOI ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOIEN; }
    else if( port == GPIOJ ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOJEN; }
    else if( port == GPIOK ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOKEN; }
    else { /* invalid port — no action */ }

    ( void )RCC->AHB4ENR;
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
    else { /* invalid port — no action */ }
}

static inline void rcc_uart_clk_enable( USART_TypeDef * uart )
{
    if( uart == USART2 ) { RCC->APB1LENR |= RCC_APB1LENR_USART2EN; }
    else if( uart == USART3 ) { RCC->APB1LENR |= RCC_APB1LENR_USART3EN; }
    else if( uart == UART4  ) { RCC->APB1LENR |= RCC_APB1LENR_UART4EN;  }
    else if( uart == UART5  ) { RCC->APB1LENR |= RCC_APB1LENR_UART5EN;  }
    else if( uart == UART7  ) { RCC->APB1LENR |= RCC_APB1LENR_UART7EN;  }
    else if( uart == UART8  ) { RCC->APB1LENR |= RCC_APB1LENR_UART8EN;  }
    else if( uart == USART1 ) { RCC->APB2ENR  |= RCC_APB2ENR_USART1EN;  }
    else if( uart == USART6 ) { RCC->APB2ENR  |= RCC_APB2ENR_USART6EN;  }
    else { /* invalid uart — no action */ }

    ( void )RCC->APB1LENR;
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
    if( tim == TIM2  ) { RCC->APB1LENR |= RCC_APB1LENR_TIM2EN;  }
    else if( tim == TIM3  ) { RCC->APB1LENR |= RCC_APB1LENR_TIM3EN;  }
    else if( tim == TIM4  ) { RCC->APB1LENR |= RCC_APB1LENR_TIM4EN;  }
    else if( tim == TIM5  ) { RCC->APB1LENR |= RCC_APB1LENR_TIM5EN;  }
    else if( tim == TIM6  ) { RCC->APB1LENR |= RCC_APB1LENR_TIM6EN;  }
    else if( tim == TIM7  ) { RCC->APB1LENR |= RCC_APB1LENR_TIM7EN;  }
    else if( tim == TIM12 ) { RCC->APB1LENR |= RCC_APB1LENR_TIM12EN; }
    else if( tim == TIM13 ) { RCC->APB1LENR |= RCC_APB1LENR_TIM13EN; }
    else if( tim == TIM14 ) { RCC->APB1LENR |= RCC_APB1LENR_TIM14EN; }
    else if( tim == TIM1  ) { RCC->APB2ENR  |= RCC_APB2ENR_TIM1EN;   }
    else if( tim == TIM8  ) { RCC->APB2ENR  |= RCC_APB2ENR_TIM8EN;   }
    else if( tim == TIM15 ) { RCC->APB2ENR  |= RCC_APB2ENR_TIM15EN;  }
    else if( tim == TIM16 ) { RCC->APB2ENR  |= RCC_APB2ENR_TIM16EN;  }
    else if( tim == TIM17 ) { RCC->APB2ENR  |= RCC_APB2ENR_TIM17EN;  }
    else { /* invalid timer — no action */ }

    ( void )RCC->APB1LENR;
}

static inline void rcc_spi_clk_enable( SPI_TypeDef * spi )
{
    if( spi == SPI1 ) { RCC->APB2ENR  |= RCC_APB2ENR_SPI1EN;  }
    else if( spi == SPI2 ) { RCC->APB1LENR |= RCC_APB1LENR_SPI2EN; }
    else if( spi == SPI3 ) { RCC->APB1LENR |= RCC_APB1LENR_SPI3EN; }
    else if( spi == SPI4 ) { RCC->APB2ENR  |= RCC_APB2ENR_SPI4EN;  }
    else if( spi == SPI5 ) { RCC->APB2ENR  |= RCC_APB2ENR_SPI5EN;  }
    else if( spi == SPI6 ) { RCC->APB4ENR  |= RCC_APB4ENR_SPI6EN;  }
    else { /* invalid spi — no action */ }

    ( void )RCC->APB2ENR;
}

static inline void rcc_i2c_clk_enable( I2C_TypeDef * i2c )
{
    if( i2c == I2C1 ) { RCC->APB1LENR |= RCC_APB1LENR_I2C1EN; }
    else if( i2c == I2C2 ) { RCC->APB1LENR |= RCC_APB1LENR_I2C2EN; }
    else if( i2c == I2C3 ) { RCC->APB1LENR |= RCC_APB1LENR_I2C3EN; }
    else if( i2c == I2C4 ) { RCC->APB4ENR  |= RCC_APB4ENR_I2C4EN;  }
    else { /* invalid i2c — no action */ }

    ( void )RCC->APB1LENR;
}

static inline void rcc_dma_clk_enable( DMA_TypeDef * dma )
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

static inline uint32_t rcc_get_usart3_clk_freq( void )
{
    /* USART3 kernel clock configured to HSI in system_init */
    return 64000000UL;  /* 64MHz */
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

static inline uint32_t rcc_get_pclk2_freq( void )
{
    uint32_t const d2ppre2 = ( RCC->D2CFGR & RCC_D2CFGR_D2PPRE2_Msk )
                               >> RCC_D2CFGR_D2PPRE2_Pos;
    uint32_t const shift   = ( ( d2ppre2 & 0x4U ) != 0U )
                               ? ( ( d2ppre2 & 0x3U ) + 1U )
                               : 0U;
    return rcc_get_hclk_freq() >> shift;
}

#endif /* HAL_RCC_H */
