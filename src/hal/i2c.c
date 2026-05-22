#include "i2c.h"
#include "lib/time/delay.h"

static result_t i2c_wait_flag( const I2C_TypeDef * p_i2c,
                               uint32_t flag,
                               uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_i2c != NULL )
    {
        delay_timer_t timer;
        result = RES_ERR_TIMEOUT;
        delay_timer_start( &timer, timeout_ms );

        while( !delay_timer_expired( &timer ) )
        {
            if( ( p_i2c->ISR & flag ) != 0U )
            {
                result = RES_OK;
                break;
            }
        }
    }

    return result;
}

static result_t i2c_wait_flag_clear( const I2C_TypeDef * p_i2c,
                                     uint32_t flag,
                                     uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_i2c != NULL )
    {
        delay_timer_t timer;
        result = RES_ERR_TIMEOUT;
        delay_timer_start( &timer, timeout_ms );

        while( !delay_timer_expired( &timer ) )
        {
            if( ( p_i2c->ISR & flag ) == 0U )
            {
                result = RES_OK;
                break;
            }
        }
    }

    return result;
}

static result_t i2c_check_errors( const I2C_TypeDef * p_i2c )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_i2c != NULL )
    {
        if( ( p_i2c->ISR & ( I2C_ISR_BERR
                           | I2C_ISR_ARLO
                           | I2C_ISR_OVR
                           | I2C_ISR_NACKF ) ) != 0U )
        {
            result = RES_ERR_BUS;
        }
        else
        {
            result = RES_OK;
        }
    }

    return result;
}

static void i2c_clear_errors( I2C_TypeDef *p_i2c )
{
    if( p_i2c != NULL )
    {
        p_i2c->ICR = I2C_ICR_BERRCF
                   | I2C_ICR_ARLOCF
                   | I2C_ICR_OVRCF
                   | I2C_ICR_NACKCF
                   | I2C_ICR_STOPCF;
    }
}

static result_t i2c_start( I2C_TypeDef * p_i2c,
                           uint8_t dev_addr,
                           uint32_t nbytes,
                           bool read,
                           uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_i2c != NULL )
    {
        /* 7-bit address, nbytes, direction */
        p_i2c->CR2 = ( ( uint32_t )dev_addr << 1U )
                   | ( nbytes << I2C_CR2_NBYTES_Pos )
                   | ( read ? I2C_CR2_RD_WRN : 0U )
                   | I2C_CR2_AUTOEND
                   | I2C_CR2_START;

        /* wait for busy — START condition sent */
        result = i2c_wait_flag_clear( p_i2c, I2C_ISR_BUSY, timeout_ms );
    }

    return result;
}

result_t i2c_init( I2C_TypeDef * p_i2c, uint32_t timing )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_i2c != NULL )
    {
        uint32_t timeout = 1000U;

        p_i2c->CR1 &= ~I2C_CR1_PE;

        /* PE must read back 0 before TIMINGR is writable */
        while( ( ( p_i2c->CR1 & I2C_CR1_PE ) != 0U ) && ( timeout > 0U ) )
        {
            --timeout;
        }

        if( timeout == 0U )
        {
            result = RES_ERR_TIMEOUT;
        }
        else
        {
            p_i2c->TIMINGR = timing;
            p_i2c->CR1 = I2C_CR1_PE;
            result = RES_OK;
        }
    }

    return result;
}

result_t i2c_write_byte( I2C_TypeDef * p_i2c,
                         uint8_t dev_addr,
                         uint8_t data,
                         uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_i2c != NULL )
    {
        i2c_clear_errors( p_i2c );

        /* start with 1 byte write */
        p_i2c->CR2 = ( ( uint32_t )dev_addr << 1U )
                   | ( 1UL << I2C_CR2_NBYTES_Pos )
                   | I2C_CR2_AUTOEND
                   | I2C_CR2_START;

        /* wait for TXIS — ready to send byte */
        result = i2c_wait_flag( p_i2c, I2C_ISR_TXIS, timeout_ms );

        if( RES_IS_OK( result ) )
        {
            p_i2c->TXDR = ( uint32_t )data;

            /* wait for STOPF — transfer complete */
            result = i2c_wait_flag( p_i2c, I2C_ISR_STOPF, timeout_ms );
        }

        if( RES_IS_OK( result ) )
        {
            result = i2c_check_errors( p_i2c );
        }

        i2c_clear_errors( p_i2c );
    }

    return result;
}

result_t i2c_write_buf( I2C_TypeDef * p_i2c,
                        uint8_t dev_addr,
                        const uint8_t * p_buf,
                        uint32_t len,
                        uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_i2c != NULL ) && ( p_buf != NULL ) && ( len > 0U ) )
    {
        uint32_t i = 0U;

        i2c_clear_errors( p_i2c );

        p_i2c->CR2 = ( ( uint32_t )dev_addr << 1U )
                   | ( len << I2C_CR2_NBYTES_Pos )
                   | I2C_CR2_AUTOEND
                   | I2C_CR2_START;

        result = RES_OK;

        while( ( i < len ) && RES_OK( result ) )
        {
            result = i2c_wait_flag( p_i2c, I2C_ISR_TXIS, timeout_ms );

            if( RES_OK( result ) )
            {
                p_i2c->TXDR = ( uint32_t )p_buf[ i ];
                i++;
            }
        }

        if( RES_OK( result ) )
        {
            result = i2c_wait_flag( p_i2c, I2C_ISR_STOPF, timeout_ms );
        }

        if( RES_OK( result ) )
        {
            result = i2c_check_errors( p_i2c );
        }

        i2c_clear_errors( p_i2c );
    }

    return result;
}

result_t i2c_read_byte( I2C_TypeDef * p_i2c,
                        uint8_t dev_addr,
                        uint8_t * p_data,
                        uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_i2c != NULL ) && ( p_data != NULL ) )
    {
        i2c_clear_errors( p_i2c );

        p_i2c->CR2 = ( ( uint32_t )dev_addr << 1U )
                   | ( 1UL << I2C_CR2_NBYTES_Pos )
                   | I2C_CR2_AUTOEND
                   | I2C_CR2_RD_WRN
                   | I2C_CR2_START;

        result = i2c_wait_flag( p_i2c, I2C_ISR_RXNE, timeout_ms );

        if( RES_OK( result ) )
        {
            *p_data = ( uint8_t )( p_i2c->RXDR & 0xFFU );
            result  = i2c_wait_flag( p_i2c, I2C_ISR_STOPF, timeout_ms );
        }

        if( RES_OK( result ) )
        {
            result = i2c_check_errors( p_i2c );
        }

        i2c_clear_errors( p_i2c );
    }

    return result;
}

result_t i2c_read_buf( I2C_TypeDef * p_i2c,
                       uint8_t dev_addr,
                       uint8_t * p_buf,
                       uint32_t len,
                       uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_i2c != NULL ) && ( p_buf != NULL ) && ( len > 0U ) )
    {
        uint32_t i = 0U;

        i2c_clear_errors( p_i2c );

        p_i2c->CR2 = ( ( uint32_t )dev_addr << 1U )
                   | ( len << I2C_CR2_NBYTES_Pos )
                   | I2C_CR2_AUTOEND
                   | I2C_CR2_RD_WRN
                   | I2C_CR2_START;

        result = RES_OK;

        while( ( i < len ) && RES_IS_OK( result ) )
        {
            result = i2c_wait_flag( p_i2c, I2C_ISR_RXNE, timeout_ms );

            if( RES_IS_OK( result ) )
            {
                p_buf[ i ] = ( uint8_t )( p_i2c->RXDR & 0xFFU );
                i++;
            }
        }

        if( RES_IS_OK( result ) )
        {
            result = i2c_wait_flag( p_i2c, I2C_ISR_STOPF, timeout_ms );
        }

        if( RES_IS_OK( result ) )
        {
            result = i2c_check_errors( p_i2c );
        }

        i2c_clear_errors( p_i2c );
    }

    return result;
}
