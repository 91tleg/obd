#include "drivers/display/hd44780/hd44780.h"
#include "drivers/display/hd44780/hd44780_i2c.h"
#include <stddef.h>
#include "lib/time/delay.h"

/* PCF8574 bit positions */
#define PCF_RS           ( 0x01U )
#define PCF_RW           ( 0x02U )
#define PCF_EN           ( 0x04U )
#define PCF_BL           ( 0x08U )

/* HD44780 commands */
#define CMD_CLEAR        ( 0x01U )
#define CMD_HOME         ( 0x02U )
#define CMD_ENTRY_MODE   ( 0x06U )
#define CMD_DISPLAY_ON   ( 0x0CU )
#define CMD_DISPLAY_OFF  ( 0x08U )
#define CMD_FUNCTION     ( 0x28U )
#define CMD_SET_DDRAM    ( 0x80U )

/* row addresses */
#define ROW0_ADDR        ( 0x00U )
#define ROW1_ADDR        ( 0x40U )

/* timing */
#define POWER_ON_MS      ( 50U )
#define INIT_MS          ( 5U )
#define CMD_MS           ( 2U )
#define CLEAR_MS         ( 2U )
#define EN_PULSE_US      ( 1U )
#define I2C_TIMEOUT_MS   ( 10U )

static uint8_t bl_flag( hd44780_t const * p_dev )
{
    return p_dev->backlight ? PCF_BL : 0U;
}

static result_t i2c_write_byte( hd44780_t const * p_dev, uint8_t data )
{
    return hd44780_i2c_write( p_dev->dev_addr, data, I2C_TIMEOUT_MS );
}

static result_t en_pulse( hd44780_t const * p_dev, uint8_t data )
{
    result_t result = i2c_write_byte( p_dev, data | PCF_EN );

    if( RES_IS_OK( result ) )
    {
        delay_us( EN_PULSE_US );
        result = i2c_write_byte( p_dev, data & ( uint8_t )( ~PCF_EN ) );
    }

    return result;
}

static result_t write_nibble( hd44780_t const * p_dev,
                              uint8_t nibble,
                              bool is_data )
{
    uint8_t const data = ( nibble & 0xF0U )
                       | bl_flag( p_dev )
                       | ( is_data ? PCF_RS : 0U );

    return en_pulse( p_dev, data );
}

static result_t write_byte( hd44780_t const * p_dev,
                            uint8_t byte,
                            bool is_data )
{
    uint8_t const high = byte & 0xF0U;
    uint8_t const low  = ( uint8_t )( byte << 4U ) & 0xF0U;
    result_t result = write_nibble( p_dev, high, is_data );

    if( RES_IS_OK( result ) )
    {
        result = write_nibble( p_dev, low, is_data );
    }

    return result;
}

static result_t send_cmd( hd44780_t const * p_dev, uint8_t cmd )
{
    result_t result = write_byte( p_dev, cmd, false );

    if( RES_IS_OK( result ) )
    {
        delay_ms( CMD_MS );
    }

    return result;
}

static result_t send_data( hd44780_t const * p_dev, uint8_t data )
{
    result_t result = write_byte( p_dev, data, true );

    if( RES_IS_OK( result ) )
    {
        delay_ms( 1U );
    }

    return result;
}

result_t hd44780_init( hd44780_t * p_dev, uint8_t dev_addr )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_dev != NULL )
    {
        p_dev->dev_addr    = dev_addr;
        p_dev->backlight   = true;
        p_dev->initialized = false;

        delay_ms( POWER_ON_MS );

        result = write_nibble( p_dev, 0x30U, false );

        if( RES_IS_OK( result ) )
        {
            delay_ms( INIT_MS );
            result = write_nibble( p_dev, 0x30U, false );
        }

        if( RES_IS_OK( result ) )
        {
            delay_ms( INIT_MS );
            result = write_nibble( p_dev, 0x30U, false );
        }

        if( RES_IS_OK( result ) )
        {
            delay_ms( INIT_MS );
            result = write_nibble( p_dev, 0x20U, false );
        }

        if( RES_IS_OK( result ) )
        {
            delay_ms( INIT_MS );
            result = send_cmd( p_dev, CMD_FUNCTION );
        }

        if( RES_IS_OK( result ) )
        {
            result = send_cmd( p_dev, CMD_DISPLAY_OFF );
        }

        if( RES_IS_OK( result ) )
        {
            result = hd44780_clear( p_dev );
        }

        if( RES_IS_OK( result ) )
        {
            result = send_cmd( p_dev, CMD_ENTRY_MODE );
        }

        if( RES_IS_OK( result ) )
        {
            result = send_cmd( p_dev, CMD_DISPLAY_ON );
        }

        if( RES_IS_OK( result ) )
        {
            p_dev->initialized = true;
        }
    }

    return result;
}

result_t hd44780_clear( hd44780_t const * p_dev )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_dev != NULL )
    {
        result = write_byte( p_dev, CMD_CLEAR, false );

        if( RES_IS_OK( result ) )
        {
            delay_ms( CLEAR_MS );
        }
    }

    return result;
}

result_t hd44780_home( hd44780_t const * p_dev )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_dev != NULL )
    {
        result = send_cmd( p_dev, CMD_HOME );
    }

    return result;
}

result_t hd44780_display_on( hd44780_t const * p_dev )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_dev != NULL )
    {
        result = send_cmd( p_dev, CMD_DISPLAY_ON );
    }

    return result;
}

result_t hd44780_display_off( hd44780_t const * p_dev )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_dev != NULL )
    {
        result = send_cmd( p_dev, CMD_DISPLAY_OFF );
    }

    return result;
}

result_t hd44780_backlight( hd44780_t * p_dev, bool on )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_dev != NULL )
    {
        p_dev->backlight = on;
        result = i2c_write_byte( p_dev, bl_flag( p_dev ) );
    }

    return result;
}

result_t hd44780_set_cursor( hd44780_t const * p_dev,
                             uint8_t col,
                             hd44780_row_t row )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_dev != NULL ) && ( col < HD44780_COLS ) )
    {
        uint8_t const row_addr = ( row == HD44780_ROW_1 )
                                 ? ROW1_ADDR
                                 : ROW0_ADDR;

        result = send_cmd( p_dev, CMD_SET_DDRAM | row_addr | col );
    }

    return result;
}

result_t hd44780_write_char( hd44780_t const * p_dev, char c )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_dev != NULL )
    {
        result = send_data( p_dev, ( uint8_t )c );
    }

    return result;
}

result_t hd44780_write_str( hd44780_t const * p_dev,
                            char const * p_str,
                            uint32_t max_len )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_dev != NULL ) && ( p_str != NULL ) &&
        ( max_len > 0U ) )
    {
        uint32_t i = 0U;
        result = RES_OK;

        while( ( i < max_len ) && ( p_str[ i ] != '\0' ) &&
               ( RES_IS_OK( result ) ) )
        {
            result = send_data( p_dev, ( uint8_t )p_str[ i ] );
            ++i;
        }
    }

    return result;
}
