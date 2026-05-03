#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stm32h753xx.h"

typedef enum
{
    GPIO_MODE_INPUT  = 0U,
    GPIO_MODE_OUTPUT = 1U,
    GPIO_MODE_AF     = 2U,
    GPIO_MODE_ANALOG = 3U,
} gpio_mode_t;

typedef enum
{
    GPIO_OTYPE_PUSH_PULL  = 0U,
    GPIO_OTYPE_OPEN_DRAIN = 1U,
} gpio_otype_t;

typedef enum
{
    GPIO_SPEED_LOW       = 0U,
    GPIO_SPEED_MEDIUM    = 1U,
    GPIO_SPEED_HIGH      = 2U,
    GPIO_SPEED_VERY_HIGH = 3U,
} gpio_speed_t;

typedef enum
{
    GPIO_PULL_NONE = 0U,
    GPIO_PULL_UP   = 1U,
    GPIO_PULL_DOWN = 2U,
} gpio_pull_t;

static inline void gpio_set_mode( GPIO_TypeDef * port, uint8_t pin,
                                  gpio_mode_t mode )
{
    if( ( port != NULL ) && ( pin < 16U ) )
    {
        uint32_t const shift = ( uint32_t )pin * 2U;
        port->MODER &= ~(  3UL << shift );
        port->MODER |=  ( ( uint32_t )mode << shift );
    }
}

static inline void gpio_set_otype( GPIO_TypeDef * port, uint8_t pin,
                                   gpio_otype_t otype )
{
    if( ( port != NULL ) && ( pin < 16U ) )
    {
        uint32_t const shift = ( uint32_t )pin;
        port->OTYPER &= ~(  1UL << shift );
        port->OTYPER |=  ( ( uint32_t )otype << shift );
    }
}

static inline void gpio_set_speed( GPIO_TypeDef * port, uint8_t pin,
                                   gpio_speed_t speed )
{
    if( ( port != NULL ) && ( pin < 16U ) )
    {
        uint32_t const shift = ( uint32_t )pin * 2U;
        port->OSPEEDR &= ~(  3UL << shift );
        port->OSPEEDR |=  ( ( uint32_t )speed << shift );
    }
}

static inline void gpio_set_pull( GPIO_TypeDef * port, uint8_t pin,
                                  gpio_pull_t pull )
{
    if( ( port != NULL ) && ( pin < 16U ) )
    {
        uint32_t const shift = ( uint32_t )pin * 2U;
        port->PUPDR &= ~(  3UL << shift );
        port->PUPDR |=  ( ( uint32_t )pull << shift );
    }
}

static inline void gpio_set_af( GPIO_TypeDef * port, uint8_t pin, uint8_t af )
{
    if( ( port != NULL ) && ( pin < 16U ) && ( af <= 15U ) )
    {
        if( pin < 8U )
        {
            uint32_t const shift = ( uint32_t )pin * 4U;
            port->AFR[ 0U ] &= ~(  0xFUL << shift );
            port->AFR[ 0U ] |=  ( ( uint32_t )af  << shift );
        }
        else
        {
            uint32_t const shift = ( ( uint32_t )pin - 8U ) * 4U;
            port->AFR[ 1U ] &= ~(  0xFUL << shift );
            port->AFR[ 1U ] |=  ( ( uint32_t )af  << shift );
        }
    }
}

static inline void gpio_set( GPIO_TypeDef * port, uint8_t pin )
{
    if( ( port != NULL ) && ( pin < 16U ) )
    {
        /* BSRR write is atomic */
        port->BSRR = ( 1UL << ( uint32_t )pin );
    }
}

static inline void gpio_clear( GPIO_TypeDef * port, uint8_t pin )
{
    if( ( port != NULL ) && ( pin < 16U ) )
    {
        /* upper 16 bits of BSRR are reset bits */
        port->BSRR = ( 1UL << ( ( uint32_t )pin + 16U ) );
    }
}

static inline void gpio_toggle( GPIO_TypeDef * port, uint8_t pin )
{
    if( ( port != NULL ) && ( pin < 16U ) )
    {
        port->ODR ^= ( 1UL << ( uint32_t )pin );
    }
}

static inline void gpio_write( GPIO_TypeDef * port, uint8_t pin, bool state )
{
    if( state )
    {
        gpio_set( port, pin );
    }
    else
    {
        gpio_clear( port, pin );
    }
}

static inline bool gpio_read( const GPIO_TypeDef * port, uint8_t pin )
{
    bool result = false;

    if( ( port != NULL ) && ( pin < 16U ) )
    {
        result = ( ( port->IDR >> ( uint32_t )pin ) & 1UL ) == 1UL;
    }

    return result;
}

#endif /* HAL_GPIO_H */
