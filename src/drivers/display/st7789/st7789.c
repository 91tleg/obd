/**
 * @file    st7789.c
 * @brief   ST7789 240x240 TFT driver implementation
 *
 *          Hardware access via st7789_spi.h.
 */

#include "drivers/display/st7789/st7789.h"
#include "drivers/display/st7789/st7789_spi.h"
#include "drivers/display/st7789/font.h"
#include <stddef.h>
#include "lib/string/str.h"

#define ST7789_NOP        ( 0x00U )
#define ST7789_SWRESET    ( 0x01U )
#define ST7789_SLPIN      ( 0x10U )
#define ST7789_SLPOUT     ( 0x11U )
#define ST7789_NORON      ( 0x13U )
#define ST7789_INVOFF     ( 0x20U )
#define ST7789_INVON      ( 0x21U )
#define ST7789_DISPOFF    ( 0x28U )
#define ST7789_DISPON     ( 0x29U )
#define ST7789_CASET      ( 0x2AU )   /* column address set              */
#define ST7789_RASET      ( 0x2BU )   /* row address set                 */
#define ST7789_RAMWR      ( 0x2CU )   /* memory write                    */
#define ST7789_MADCTL     ( 0x36U )   /* memory data access control      */
#define ST7789_COLMOD     ( 0x3AU )   /* interface pixel format          */
#define ST7789_PORCTRL    ( 0xB2U )   /* porch setting                   */
#define ST7789_GCTRL      ( 0xB7U )   /* gate control                    */
#define ST7789_VCOMS      ( 0xBBU )   /* VCOM setting                    */
#define ST7789_LCMCTRL    ( 0xC0U )
#define ST7789_VDVVRHEN   ( 0xC2U )
#define ST7789_VRHS       ( 0xC3U )
#define ST7789_VDVS       ( 0xC4U )
#define ST7789_FRCTRL2    ( 0xC6U )
#define ST7789_PWCTRL1    ( 0xD0U )
#define ST7789_PVGAMCTRL  ( 0xE0U )   /* positive voltage gamma control  */
#define ST7789_NVGAMCTRL  ( 0xE1U )   /* negative voltage gamma control  */

/* MADCTL bits */
#define MADCTL_MY   ( 0x80U )
#define MADCTL_MX   ( 0x40U )
#define MADCTL_MV   ( 0x20U )
#define MADCTL_ML   ( 0x10U )
#define MADCTL_RGB  ( 0x00U )
#define MADCTL_BGR  ( 0x08U )

static result_t write_cmd( uint8_t cmd )
{
    st7789_dc_set( false );
    st7789_cs_set( false );
    result_t result = st7789_spi_write_byte( cmd );
    st7789_cs_set( true );
    return result;
}

static result_t write_data_u8( uint8_t data )
{
    st7789_dc_set( true );
    st7789_cs_set( false );
    result_t result = st7789_spi_write_byte( data );
    st7789_cs_set( true );
    return result;
}

static result_t write_data_u16( uint16_t data )
{
    uint8_t buf[ 2U ];
    buf[ 0U ] = ( uint8_t )( data >> 8U );
    buf[ 1U ] = ( uint8_t )( data & 0xFFU );

    st7789_dc_set( true );
    st7789_cs_set( false );
    result_t result = st7789_spi_write_buf( buf, 2U );
    st7789_cs_set( true );
    return result;
}

static result_t write_data_buf( uint8_t const * buf, uint32_t len )
{
    st7789_dc_set( true );
    st7789_cs_set( false );
    result_t result = st7789_spi_write_buf( buf, len );
    st7789_cs_set( true );
    return result;
}

static result_t set_window( uint16_t x0, uint16_t y0,
                            uint16_t x1, uint16_t y1 )
{
    uint8_t buf[ 4U ];

    buf[ 0U ] = ( uint8_t )( x0 >> 8U );
    buf[ 1U ] = ( uint8_t )( x0 & 0xFFU );
    buf[ 2U ] = ( uint8_t )( x1 >> 8U );
    buf[ 3U ] = ( uint8_t )( x1 & 0xFFU );

    result_t result = write_cmd( ST7789_CASET );
    if( RES_IS_OK( result ) )
    {
        result = write_data_buf( buf, 4U );
    }

    buf[ 0U ] = ( uint8_t )( y0 >> 8U );
    buf[ 1U ] = ( uint8_t )( y0 & 0xFFU );
    buf[ 2U ] = ( uint8_t )( y1 >> 8U );
    buf[ 3U ] = ( uint8_t )( y1 & 0xFFU );

    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_RASET );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_buf( buf, 4U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_RAMWR );
    }

    return result;
}

result_t st7789_init( void )
{
    /* hardware reset */
    st7789_bl_set( false );
    st7789_cs_set( true );
    st7789_dc_set( false );
    st7789_rst_set( false );
    st7789_delay_ms( 10U );
    st7789_rst_set( true );
    st7789_delay_ms( 120U );

    /* software reset */
    result_t result = write_cmd( ST7789_SWRESET );
    st7789_delay_ms( 150U );

    /* sleep out */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_SLPOUT );
    }
    st7789_delay_ms( 10U );

    /* pixel format — 16-bit RGB565 */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_COLMOD );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x55U );
    }

    /* memory access — portrait, RGB order */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_MADCTL );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( MADCTL_RGB );
    }

    /* porch control */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_PORCTRL );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x0CU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x0CU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x00U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x33U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x33U );
    }

    /* gate control */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_GCTRL );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x35U );
    }

    /* VCOM */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_VCOMS );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x19U );
    }

    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_LCMCTRL );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x2CU );
    }

    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_VDVVRHEN );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x01U );
    }

    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_VRHS );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x12U );
    }

    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_VDVS );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x20U );
    }

    /* frame rate 60 Hz */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_FRCTRL2 );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x0FU );
    }

    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_PWCTRL1 );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0xA4U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0xA1U );
    }

    /* positive gamma */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_PVGAMCTRL );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0xD0U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x04U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x0DU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x11U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x13U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x2BU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x3FU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x54U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x4CU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x18U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x0DU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x0BU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x1FU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x23U );
    }

    /* negative gamma */
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_NVGAMCTRL );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0xD0U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x04U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x0CU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x11U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x13U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x2CU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x3FU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x44U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x51U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x2FU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x1FU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x1FU );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x20U );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_data_u8( 0x23U );
    }

    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_INVON );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_NORON );
    }
    if( RES_IS_OK( result ) )
    {
        result = write_cmd( ST7789_DISPON );
    }
    st7789_delay_ms( 5U );

    if( RES_IS_OK( result ) )
    {
        st7789_bl_set( true );
    }

    return result;
}

result_t st7789_fill( uint16_t color )
{
    uint32_t i = 0U;
    uint16_t x_end  = ( uint16_t )ST7789_WIDTH  - 1U;
    uint16_t y_end  = ( uint16_t )ST7789_HEIGHT - 1U;
    uint32_t total  = ( uint32_t )ST7789_WIDTH * ( uint32_t )ST7789_HEIGHT;
    result_t result = set_window( 0U, 0U, x_end, y_end );

    while( RES_IS_OK( result ) && ( i < total ) )
    {
        result = write_data_u16( color );
        ++i;
    }
    return result;
}

result_t st7789_draw_char( uint16_t x,
                           uint16_t y,
                           char     c,
                           uint16_t fg,
                           uint16_t bg )
{
    result_t result = RES_OK;
    bool valid = true;

    if( ( ( uint32_t )x + ( uint32_t )FONT7X10_W ) >
        ( uint32_t )ST7789_WIDTH )
    {
        valid = false;
    }

    if( ( ( uint32_t )y + ( uint32_t )FONT7X10_H ) >
        ( uint32_t )ST7789_HEIGHT )
    {
        valid = false;
    }

    if( ( ( uint8_t )c < ( uint8_t )FONT7X10_FIRST ) ||
        ( ( uint8_t )c > ( uint8_t )FONT7X10_LAST ) )
    {
        valid = false;
    }

    if( valid )
    {
        uint8_t char_index = 0U;
        uint16_t glyph_idx = 0U;
        uint16_t pixel;
        uint32_t row = 0U;

        char_index =
            ( uint8_t )c - ( uint8_t )FONT7X10_FIRST;

        glyph_idx =
            ( uint16_t )char_index * ( uint16_t )FONT7X10_H;

        result = set_window(
            x,
            y,
            ( uint16_t )( x + ( uint16_t )FONT7X10_W - 1U ),
            ( uint16_t )( y + ( uint16_t )FONT7X10_H - 1U ) );

        row = 0U;

        while( RES_IS_OK( result ) &&
               ( row < ( uint32_t )FONT7X10_H ) )
        {
            uint16_t row_data = 0U;
            uint32_t col = 0U;

            row_data = Font7x10[ glyph_idx + ( uint16_t )row ];

            while( RES_IS_OK( result ) &&
                   ( col < ( uint32_t )FONT7X10_W ) )
            {
                if( ( row_data &
                      ( uint16_t )( 0x8000U >> col ) ) != 0U )
                {
                    pixel = fg;
                }
                else
                {
                    pixel = bg;
                }

                result = write_data_u16( pixel );
                ++col;
            }

            ++row;
        }
    }

    return result;
}

result_t st7789_draw_str( uint16_t x,
                          uint16_t y,
                          char const * str,
                          uint16_t fg,
                          uint16_t bg )
{
    uint16_t cur_x = x;
    uint16_t cur_y = y;
    uint32_t i = 0U;
    uint32_t len = 0U;
    bool running = true;
    result_t result = RES_OK;

    if( str == NULL )
    {
        result = RES_ERR_INVALID_ARG;
    }
    else
    {
        len = str_len( str );
    }

    while( RES_IS_OK( result ) && running && ( i < len ) )
    {
        if( ( ( uint32_t )cur_x + ( uint32_t )FONT7X10_W ) >
            ( uint32_t )ST7789_WIDTH )
        {
            cur_x = 0U;
            cur_y = ( uint16_t )( cur_y + ( uint16_t )FONT7X10_H );
        }

        if( ( ( uint32_t )cur_y + ( uint32_t )FONT7X10_H ) >
            ( uint32_t )ST7789_HEIGHT )
        {
            running = false;
        }
        else
        {
            result = st7789_draw_char( cur_x,
                                       cur_y,
                                       str[ i ],
                                       fg,
                                       bg );

            cur_x = ( uint16_t )( cur_x +
                                  ( uint16_t )FONT7X10_W );
            ++i;
        }
    }

    return result;
}
