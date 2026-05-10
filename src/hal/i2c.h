#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>
#include <stddef.h>
#include "stm32h753xx.h"
#include "lib/core/result.h"

static inline bool i2c_busy( const I2C_TypeDef * p_i2c )
{
    bool result = false;

    if( p_i2c != NULL )
    {
        result = ( ( p_i2c->ISR & I2C_ISR_BUSY ) != 0U );
    }

    return result;
}

static inline void i2c_enable( I2C_TypeDef * p_i2c )
{
    if( p_i2c != NULL )
    {
        p_i2c->CR1 |= I2C_CR1_PE;
    }
}

static inline void i2c_disable( I2C_TypeDef * p_i2c )
{
    if( p_i2c != NULL )
    {
        p_i2c->CR1 &= ~I2C_CR1_PE;
    }
}

result_t i2c_init( I2C_TypeDef * p_i2c, uint32_t timing );

result_t i2c_write_byte( I2C_TypeDef * p_i2c, uint8_t dev_addr,
                         uint8_t data, uint32_t timeout_ms );

result_t i2c_write_buf( I2C_TypeDef * p_i2c, uint8_t dev_addr,
                        const uint8_t * p_buf, uint32_t len,
                        uint32_t timeout_ms );

result_t i2c_read_byte( I2C_TypeDef * p_i2c, uint8_t dev_addr,
                        uint8_t * p_data, uint32_t timeout_ms );

result_t i2c_read_buf( I2C_TypeDef * p_i2c, uint8_t dev_addr,
                       uint8_t * p_buf, uint32_t len,
                       uint32_t timeout_ms );

#endif /* HAL_I2C_H */
