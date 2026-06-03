#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include "lib/core/result.h"
#include "drivers/protocol/obd/obd_types.h"

typedef enum
{
    PAGE_LIVE_DATA = 0U,
    PAGE_DTCS      = 1U,
    PAGE_CLEAR     = 2U,
    PAGE_COUNT     = 3U,
} display_page_t;

result_t display_app_init( void );
void display_app_next_page( void );
display_page_t display_app_get_page( void );
void display_app_invalidate( void );
result_t display_app_update( live_data_t const * live,
                             dtc_list_t  const * dtcs );
result_t display_app_show_splash( char const * row0,
                                  char const * row1 );

#endif /* APP_DISPLAY_H */
