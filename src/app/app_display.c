#include "app/app_display.h"
#include <stddef.h>
#include "drivers/display/display.h"
#include "lib/string/str.h"
#include "lib/core/macros.h"

#define DISPLAY_BUF_LEN  ( 32U )

static display_page_t s_page = PAGE_LIVE_DATA;
static bool s_dirty    = true;
static uint32_t s_live_idx = 0U;
static uint32_t s_dtc_idx  = 0U;

typedef struct
{
    obd_param_id_t id;
    char const * label;
    char const * unit;
    uint8_t decimals;
} param_fmt_t;

static param_fmt_t const * find_fmt( obd_param_id_t id )
{
    static param_fmt_t const s_fmt[] =
    {
        { OBD_PARAM_ENGINE_LOAD,      "Load",      "%",    1U },
        { OBD_PARAM_COOLANT_TEMP,     "Coolant",   "C",    0U },
        { OBD_PARAM_SHORT_TERM_FUEL,  "STFT",      "%",    1U },
        { OBD_PARAM_LONG_TERM_FUEL,   "LTFT",      "%",    1U },
        { OBD_PARAM_INTAKE_PRESSURE,  "MAP",       "kPa",  0U },
        { OBD_PARAM_RPM,              "RPM",       "rpm",  0U },
        { OBD_PARAM_VEHICLE_SPEED,    "Speed",     "km/h", 0U },
        { OBD_PARAM_TIMING_ADVANCE,   "Timing",    "deg",  1U },
        { OBD_PARAM_INTAKE_TEMP,      "IAT",       "C",    0U },
        { OBD_PARAM_MAF,              "MAF",       "g/s",  2U },
        { OBD_PARAM_THROTTLE,         "Throttle",  "%",    1U },
        { OBD_PARAM_RUNTIME,          "Runtime",   "s",    0U },
        { OBD_PARAM_DISTANCE_MIL,     "Dist MIL",  "km",   0U },
        { OBD_PARAM_FUEL_LEVEL,       "Fuel",      "%",    1U },
        { OBD_PARAM_BAROMETRIC,       "Baro",      "kPa",  0U },
        { OBD_PARAM_CONTROL_MODULE_V, "Batt",      "V",    2U },
        { OBD_PARAM_AMBIENT_TEMP,     "Ambient",   "C",    0U },
        { OBD_PARAM_OIL_TEMP,         "Oil",       "C",    0U },
        { OBD_PARAM_FUEL_RATE,        "Fuel Rate", "L/h",  2U },
    };

    param_fmt_t const * result = NULL;
    uint32_t i = 0U;
    uint32_t const fmt_size = ARRAY_SIZE( s_fmt );

    while( ( i < fmt_size ) && ( result == NULL ) )
    {
        if( s_fmt[ i ].id == id )
        {
            result = &s_fmt[ i ];
        }
        ++i;
    }

    return result;
}

static result_t render_live( live_data_t const * live )
{
    char row0[ DISPLAY_BUF_LEN ];
    char row1[ DISPLAY_BUF_LEN ];

    if( ( live == NULL ) || ( live->count == 0U ) )
    {
        str_copy( row0, "No Data", DISPLAY_BUF_LEN );
        str_copy( row1, "---",     DISPLAY_BUF_LEN );
    }
    else
    {
        if( s_live_idx >= live->count )
        {
            s_live_idx = 0U;
        }

        live_data_param_t const * param = &live->params[ s_live_idx ];
        param_fmt_t const * fmt = find_fmt( ( obd_param_id_t )param->id );

        if( ( fmt != NULL ) && ( param->valid ) )
        {
            char val_buf[ DISPLAY_BUF_LEN ];
            str_copy( row0, fmt->label, DISPLAY_BUF_LEN );
            ( void )f32_to_str( param->value, val_buf,
                                sizeof( val_buf ), fmt->decimals );
            str_copy  ( row1, val_buf,   DISPLAY_BUF_LEN );
            str_concat( row1, " ",       DISPLAY_BUF_LEN );
            str_concat( row1, fmt->unit, DISPLAY_BUF_LEN );
        }
        else
        {
            str_copy( row0, "Unknown", DISPLAY_BUF_LEN );
            str_copy( row1, "---",     DISPLAY_BUF_LEN );
        }
    }

    result_t result = display_print( 0U, row0 );
    if( RES_IS_OK( result ) )
    {
        result = display_print( 1U, row1 );
    }
    return result;
}

static result_t render_dtc( dtc_list_t const * dtcs )
{
    char row0[ DISPLAY_BUF_LEN ];
    char row1[ DISPLAY_BUF_LEN ];

    if( ( dtcs == NULL ) || ( dtcs->count == 0U ) )
    {
        str_copy( row0, "No DTCs", DISPLAY_BUF_LEN );
        str_copy( row1, "",        DISPLAY_BUF_LEN );
    }
    else
    {
        char num_buf[ 8U ];

        if( s_dtc_idx >= dtcs->count )
        {
            s_dtc_idx = 0U;
        }

        dtc_entry_t const * dtc = &dtcs->codes[ s_dtc_idx ];

        str_copy( row0, "DTC ", DISPLAY_BUF_LEN );
        ( void )u32_to_str( s_dtc_idx + 1U, num_buf, sizeof( num_buf ) );
        str_concat( row0, num_buf, DISPLAY_BUF_LEN );
        str_concat( row0, "/",     DISPLAY_BUF_LEN );
        ( void )u32_to_str( dtcs->count, num_buf, sizeof( num_buf ) );
        str_concat( row0, num_buf, DISPLAY_BUF_LEN );

        ( void )u32_to_hex( dtc->code, row1, DISPLAY_BUF_LEN );
        if( dtc->pending )
        {
            str_concat( row1, " PND", DISPLAY_BUF_LEN );
        }
    }

    result_t result = display_print( 0U, row0 );
    if( RES_IS_OK( result ) )
    {
        result = display_print( 1U, row1 );
    }
    return result;
}

static result_t render_clear( void )
{
    result_t result = display_print( 0U, "Hold to clear" );
    if( RES_IS_OK( result ) )
    {
        result = display_print( 1U, "DTCs" );
    }
    return result;
}

result_t display_app_init( void )
{
    s_page     = PAGE_LIVE_DATA;
    s_dirty    = true;
    s_live_idx = 0U;
    s_dtc_idx  = 0U;
    return display_init();
}

void display_app_next_page( void )
{
    uint32_t const next = ( ( uint32_t )s_page + 1U ) % ( uint32_t )PAGE_COUNT;
    s_page = ( display_page_t )next;

    if( s_page == PAGE_LIVE_DATA )
    {
        s_live_idx = 0U;
    }
    if( s_page == PAGE_DTCS )
    {
        s_dtc_idx = 0U;
    }

    s_dirty = true;
}

display_page_t display_app_get_page( void )
{
    return s_page;
}

void display_app_invalidate( void )
{
    s_dirty = true;
}

result_t display_app_update( live_data_t const * live,
                             dtc_list_t  const * dtcs )
{
    result_t result = RES_OK;

    if( s_dirty )
    {
        result = display_clear();

        if( RES_IS_OK( result ) )
        {
            switch( s_page )
            {
                case PAGE_LIVE_DATA:
                    result = render_live( live );
                    break;
                case PAGE_DTCS:
                    result = render_dtc( dtcs );
                    break;
                case PAGE_CLEAR:
                    result = render_clear();
                    break;
                default:
                    break;
            }
        }

        if( RES_IS_OK( result ) )
        {
            s_dirty = false;
        }
    }

    return result;
}

result_t display_app_show_splash( char const * row0, char const * row1 )
{
    result_t result = display_clear();
    if( RES_IS_OK( result ) )
    {
        result = display_print( 0U, ( row0 != NULL ) ? row0 : "" );
    }
    if( RES_IS_OK( result ) )
    {
        result = display_print( 1U, ( row1 != NULL ) ? row1 : "" );
    }
    return result;
}
