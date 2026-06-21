#include "drivers/display/display.h"
#include "drivers/display/st7789/st7789.h"

result_t display_init( void )
{
    return st7789_init();
}

result_t display_clear( void )
{
    return st7789_fill( ST7789_BLACK );
}

result_t display_print( uint8_t row, char const * str )
{
    uint16_t y = ( uint16_t )row * ( uint16_t )FONT7X10_H;
    return st7789_draw_str( 0U, y, str, ST7789_WHITE, ST7789_BLACK );
}
