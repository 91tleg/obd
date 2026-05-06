#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h753xx.h"

typedef enum
{
    TIM_MODE_UP     = 0U,
    TIM_MODE_DOWN   = 1U,
    TIM_MODE_CENTER = 2U,
} tim_count_mode_t;

typedef enum
{
    TIM_OCM_FROZEN     = 0U,
    TIM_OCM_ACTIVE     = 1U,
    TIM_OCM_INACTIVE   = 2U,
    TIM_OCM_TOGGLE     = 3U,
    TIM_OCM_FORCE_LOW  = 4U,
    TIM_OCM_FORCE_HIGH = 5U,
    TIM_OCM_PWM1       = 6U,
    TIM_OCM_PWM2       = 7U,
} tim_ocm_t;

typedef enum
{
    TIM_CH1 = 0U,
    TIM_CH2 = 1U,
    TIM_CH3 = 2U,
    TIM_CH4 = 3U,
} tim_channel_t;

typedef enum
{
    TIM_POL_HIGH = 0U,
    TIM_POL_LOW  = 1U,
} tim_polarity_t;

static inline void tim_enable( TIM_TypeDef * p_tim )
{
    if( p_tim != NULL )
    {
        p_tim->CR1 |= TIM_CR1_CEN;
    }
}

static inline void tim_disable( TIM_TypeDef * p_tim )
{
    if( p_tim != NULL )
    {
        p_tim->CR1 &= ~TIM_CR1_CEN;
    }
}

static inline bool tim_is_enabled( const TIM_TypeDef * p_tim )
{
    bool result = false;

    if( p_tim != NULL )
    {
        result = ( ( p_tim->CR1 & TIM_CR1_CEN ) != 0U );
    }

    return result;
}

static inline void tim_set_prescaler( TIM_TypeDef * p_tim,
                                      uint32_t psc )
{
    if( p_tim != NULL )
    {
        p_tim->PSC = psc;
    }
}

static inline void tim_set_autoreload( TIM_TypeDef * p_tim,
                                       uint32_t arr )
{
    if( p_tim != NULL )
    {
        p_tim->ARR = arr;
    }
}

static inline void tim_set_count( TIM_TypeDef * p_tim,
                                  uint32_t cnt )
{
    if( p_tim != NULL )
    {
        p_tim->CNT = cnt;
    }
}

static inline uint32_t tim_get_count( const TIM_TypeDef * p_tim )
{
    uint32_t result = 0U;

    if( p_tim != NULL )
    {
        result = p_tim->CNT;
    }

    return result;
}

static inline void tim_generate_update( TIM_TypeDef * p_tim )
{
    if( p_tim != NULL )
    {
        p_tim->EGR |= TIM_EGR_UG;
    }
}

static inline void tim_set_count_mode( TIM_TypeDef * p_tim,
                                       tim_count_mode_t mode )
{
    if( p_tim != NULL )
    {
        p_tim->CR1 &= ~( TIM_CR1_DIR | TIM_CR1_CMS );

        switch( mode )
        {
            case TIM_MODE_UP:
                /* DIR=0 CMS=00 — reset default, no bits to set */
                break;

            case TIM_MODE_DOWN:
                p_tim->CR1 |= TIM_CR1_DIR;
                break;

            case TIM_MODE_CENTER:
                p_tim->CR1 |= TIM_CR1_CMS_0;
                break;

            default:
                /* invalid — leave as up count */
                break;
        }
    }
}

static inline void tim_set_arpe( TIM_TypeDef * p_tim, bool enable )
{
    if( p_tim != NULL )
    {
        if( enable )
        {
            p_tim->CR1 |= TIM_CR1_ARPE;
        }
        else
        {
            p_tim->CR1 &= ~TIM_CR1_ARPE;
        }
    }
}

static inline void tim_set_one_pulse( TIM_TypeDef * p_tim, bool enable )
{
    if( p_tim != NULL )
    {
        if( enable )
        {
            p_tim->CR1 |= TIM_CR1_OPM;
        }
        else
        {
            p_tim->CR1 &= ~TIM_CR1_OPM;
        }
    }
}

static inline void tim_set_ocm( TIM_TypeDef * p_tim,
                                 tim_channel_t ch,
                                 tim_ocm_t mode )
{
    if( p_tim != NULL )
    {
        switch( ch )
        {
            case TIM_CH1:
                p_tim->CCMR1 &= ~TIM_CCMR1_OC1M;
                p_tim->CCMR1 |=  ( ( uint32_t )mode << TIM_CCMR1_OC1M_Pos );
                break;

            case TIM_CH2:
                p_tim->CCMR1 &= ~TIM_CCMR1_OC2M;
                p_tim->CCMR1 |=  ( ( uint32_t )mode << TIM_CCMR1_OC2M_Pos );
                break;

            case TIM_CH3:
                p_tim->CCMR2 &= ~TIM_CCMR2_OC3M;
                p_tim->CCMR2 |=  ( ( uint32_t )mode << TIM_CCMR2_OC3M_Pos );
                break;

            case TIM_CH4:
                p_tim->CCMR2 &= ~TIM_CCMR2_OC4M;
                p_tim->CCMR2 |=  ( ( uint32_t )mode << TIM_CCMR2_OC4M_Pos );
                break;

            default:
                break;
        }
    }
}

static inline void tim_set_ccr( TIM_TypeDef * p_tim,
                                 tim_channel_t ch,
                                 uint32_t ccr )
{
    if( p_tim != NULL )
    {
        switch( ch )
        {
            case TIM_CH1:
                p_tim->CCR1 = ccr;
                break;

            case TIM_CH2:
                p_tim->CCR2 = ccr;
                break;

            case TIM_CH3:
                p_tim->CCR3 = ccr;
                break;

            case TIM_CH4:
                p_tim->CCR4 = ccr;
                break;

            default:
                break;
        }
    }
}

static inline uint32_t tim_get_ccr( const TIM_TypeDef * p_tim,
                                     tim_channel_t ch )
{
    uint32_t result = 0U;

    if( p_tim != NULL )
    {
        switch( ch )
        {
            case TIM_CH1:
                result = p_tim->CCR1;
                break;

            case TIM_CH2:
                result = p_tim->CCR2;
                break;

            case TIM_CH3:
                result = p_tim->CCR3;
                break;

            case TIM_CH4:
                result = p_tim->CCR4;
                break;

            default:
                break;
        }
    }

    return result;
}

static inline void tim_ch_enable( TIM_TypeDef * p_tim, tim_channel_t ch )
{
    if( p_tim != NULL )
    {
        p_tim->CCER |= ( 1UL << ( ( uint32_t )ch * 4U ) );
    }
}

static inline void tim_ch_disable( TIM_TypeDef * p_tim, tim_channel_t ch )
{
    if( p_tim != NULL )
    {
        p_tim->CCER &= ~( 1UL << ( ( uint32_t )ch * 4U ) );
    }
}

static inline void tim_ch_set_polarity( TIM_TypeDef * p_tim,
                                        tim_channel_t ch,
                                        tim_polarity_t pol )
{
    if( p_tim != NULL )
    {
        uint32_t const bit = ( ( uint32_t )ch * 4U ) + 1U;

        if( pol == TIM_POL_LOW )
        {
            p_tim->CCER |=  ( 1UL << bit );
        }
        else
        {
            p_tim->CCER &= ~( 1UL << bit );
        }
    }
}

/* caller is responsible for only passing TIM1 or TIM8 */
static inline void tim_moe_enable( TIM_TypeDef * p_tim )
{
    if( p_tim != NULL )
    {
        p_tim->BDTR |= TIM_BDTR_MOE;
    }
}

static inline void tim_enable_irq_update( TIM_TypeDef * p_tim )
{
    if( p_tim != NULL )
    {
        p_tim->DIER |= TIM_DIER_UIE;
    }
}

static inline void tim_disable_irq_update( TIM_TypeDef * p_tim )
{
    if( p_tim != NULL )
    {
        p_tim->DIER &= ~TIM_DIER_UIE;
    }
}

static inline void tim_enable_irq_cc( TIM_TypeDef * p_tim, tim_channel_t ch )
{
    if( p_tim != NULL )
    {
        p_tim->DIER |= ( 1UL << ( ( uint32_t )ch + 1U ) );
    }
}

static inline void tim_disable_irq_cc( TIM_TypeDef * p_tim, tim_channel_t ch )
{
    if( p_tim != NULL )
    {
        p_tim->DIER &= ~( 1UL << ( ( uint32_t )ch + 1U ) );
    }
}

static inline bool tim_flag_update( const TIM_TypeDef * p_tim )
{
    bool result = false;

    if( p_tim != NULL )
    {
        result = ( ( p_tim->SR & TIM_SR_UIF ) != 0U );
    }

    return result;
}

static inline bool tim_flag_cc( const TIM_TypeDef * p_tim, tim_channel_t ch )
{
    bool result = false;

    if( p_tim != NULL )
    {
        result = ( ( p_tim->SR &
                   ( 1UL << ( ( uint32_t )ch + 1U ) ) ) != 0U );
    }

    return result;
}

static inline void tim_clear_flag_update( TIM_TypeDef * p_tim )
{
    if( p_tim != NULL )
    {
        p_tim->SR &= ~TIM_SR_UIF;
    }
}

static inline void tim_clear_flag_cc( TIM_TypeDef * p_tim, tim_channel_t ch )
{
    if( p_tim != NULL )
    {
        p_tim->SR &= ~( 1UL << ( ( uint32_t )ch + 1U ) );
    }
}

static inline void tim_configure_periodic( TIM_TypeDef * p_tim,
                                           uint32_t pclk_hz,
                                           uint32_t period_us )
{
    if( ( p_tim != NULL ) && ( period_us > 0U ) && ( pclk_hz > 0U ) )
    {
        /* multiply first to preserve precision, then divide */
        uint32_t const ticks = ( uint32_t )
            ( ( ( uint64_t )pclk_hz * ( uint64_t )period_us )
              / 1000000UL );

        if( ticks > 0U )
        {
            tim_disable( p_tim );
            tim_set_count_mode( p_tim, TIM_MODE_UP );
            tim_set_prescaler( p_tim, 0U );
            tim_set_autoreload( p_tim, ticks - 1U );
            tim_set_arpe( p_tim, true );
            tim_generate_update( p_tim );
            tim_clear_flag_update( p_tim );
        }
    }
}

#endif /* HAL_TIMER_H */
