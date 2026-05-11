/**
 * @file    ema.h
 * @brief   Exponential moving average filter
 *          Integer variant - for ISR paths, no float
 *          Float variant   - for non-ISR paths
 */
 
#ifndef LIB_EMA_H
#define LIB_EMA_H

#include <stdint.h>
#include <stddef.h>

#define EMA_ALPHA_0_5   ( 128U )
#define EMA_ALPHA_0_25  ( 64U )
#define EMA_ALPHA_0_1   ( 26U )
#define EMA_INPUT_MAX   ( 0x00FFFFFFU )  /* max input before shift overflow */

/**
 * @brief Integer EMA
 * alpha is fixed point: alpha=128 means 0.5, alpha=26 means ~0.1
 */
typedef struct
{
    uint32_t value;  /* current filtered value, scaled << 8 */
    uint8_t  alpha;  /* smoothing factor 1..255 */
} ema_t;

static inline void ema_init( ema_t * p_ema, uint8_t alpha, uint32_t initial )
{
    if( p_ema != NULL )
    {
        p_ema->alpha = alpha;
        p_ema->value = ( initial & EMA_INPUT_MAX ) << 8U;
    }
}

static inline uint32_t ema_update( ema_t * p_ema, uint32_t sample )
{
    uint32_t result = 0U;

    if( p_ema != NULL )
    {
        uint32_t const alpha = ( uint32_t )p_ema->alpha;
        uint32_t const inv_alpha = 256U - alpha;
        uint32_t const prev = p_ema->value >> 8U;

        p_ema->value = ( alpha * sample ) + ( inv_alpha * prev );
        result = p_ema->value >> 8U;
    }

    return result;
}

static inline uint32_t ema_get( const ema_t * p_ema )
{
    uint32_t result = 0U;

    if( p_ema != NULL )
    {
        result = p_ema->value >> 8U;
    }

    return result;
}

/**
 * @brief float EMA
 */
typedef struct
{
    float value;
    float alpha;
} ema_f32_t;

static inline void ema_f32_init( ema_f32_t * p_ema,
                                 float alpha,
                                 float initial )
{
    if( p_ema != NULL )
    {
        p_ema->alpha = alpha;
        p_ema->value = initial;
    }
}

static inline float ema_f32_update( ema_f32_t * p_ema, float sample )
{
    float result = 0.0F;

    if( p_ema != NULL )
    {
        p_ema->value += p_ema->alpha * ( sample - p_ema->value );
        result = p_ema->value;
    }

    return result;
}

static inline float ema_f32_get( const ema_f32_t * p_ema )
{
    float result = 0.0F;

    if( p_ema != NULL )
    {
        result = p_ema->value;
    }

    return result;
}

#endif /* LIB_EMA_H */
