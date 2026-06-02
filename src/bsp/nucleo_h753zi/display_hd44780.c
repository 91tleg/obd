#include "drivers/display/display.h"
#include "drivers/display/hd44780/hd44780.h"

#define LCD_I2C_ADDR  ( 0x27U )

static hd44780_t s_display;

result_t display_init( void )
{
    return hd44780_init( &s_display, LCD_I2C_ADDR );
}

result_t display_clear( void )
{
    return hd44780_clear( &s_display );
}

result_t display_print( uint8_t row, char const * str )
{
    result_t result = hd44780_set_cursor( &s_display, 0U, ( hd44780_row_t )row );
    if( RES_IS_OK( result ) )
    {
        result = hd44780_write_str( &s_display, str, HD44780_COLS );
    }
    return result;
}
