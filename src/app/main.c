#include "bsp/bsp.h"
#include "lib/time/delay.h"
#include "lib/time/delay_tick.h"
#include "lib/log/log.h"
#include "lib/core/result.h"
#include "drivers/button/button_callback.h"
#include "app/app_display.h"
#include "app/app_obd.h"

#define TAG  "MAIN"

static volatile btn_event_t s_btn_event = BTN_EVENT_NONE;
static obd_app_t s_obd;
static uint32_t s_last_blink_ms = 0U;

static void display_update_or_halt( live_data_t const * live,
                                    dtc_list_t  const * dtcs )
{
    display_app_invalidate();
    result_t const result = display_app_update( live, dtcs );
    if( RES_IS_FAILED( result ) )
    {
        LOGE( TAG, "display fault — waiting for WDT reset" );
        for( ;; )
        {
            /* spin — WDT resets the system */
        }
    }
}

static void display_splash_or_halt( char const * row0, char const * row1 )
{
    result_t const result = display_app_show_splash( row0, row1 );
    if( RES_IS_FAILED( result ) )
    {
        LOGE( TAG, "display fault — waiting for WDT reset" );
        for( ;; )
        {
            /* spin — WDT resets the system */
        }
    }
}

void btn_event_callback( btn_event_t event )
{
    s_btn_event = event;
}

int main( void )
{
    result_t result;

    result = board_init();

    if( RES_IS_FAILED( result ) )
    {
        LOGE( TAG, "board_init failed — waiting for WDT reset" );
        for( ;; )
        {
            /* spin — WDT resets the system */
        }
    }

    delay_init( bsp_clock_sysclk_hz() );
    log_set_level( LOG_DEBUG );
    LOGI( TAG, "OBD scan tool starting" );

    bsp_wdt_refresh();

    result = display_app_init();

    if( RES_IS_FAILED( result ) )
    {
        LOGE( TAG, "display init failed" );
        for( ;; )
        {
            /* spin — WDT resets the system */
        }
    }
    else
    {
        display_splash_or_halt( " OBD Scan Tool  ", " Connecting...  " );
    }

    obd_app_init( &s_obd );

    result = obd_app_connect( &s_obd );

    if( RES_IS_OK( result ) )
    {
        LOGI( TAG, "OBD connected" );
        display_splash_or_halt( " OBD Scan Tool  ", "   Connected!   " );
        delay_ms( 1000U );
        display_app_invalidate();
    }
    else
    {
        LOGE( TAG, "OBD connect failed" );
        display_splash_or_halt( " OBD Scan Tool  ", "  No ECU found  " );
    }

    bsp_led_green_off();

    for( ;; )
    {
        bsp_wdt_refresh();

        uint32_t const now_ms = delay_get_tick();

        /* consume button event */
        btn_event_t const evt = s_btn_event;
        s_btn_event = BTN_EVENT_NONE;

        switch( evt )
        {
            case BTN_EVENT_PRESSED:
                display_app_next_page();
                LOGD( TAG, "btn: next page" );
                break;

            case BTN_EVENT_HELD:
                if( display_app_get_page() == PAGE_DTCS )
                {
                    obd_app_request_dtc_read( &s_obd );
                    LOGI( TAG, "btn: dtc read requested" );
                }
                else if( display_app_get_page() == PAGE_CLEAR )
                {
                    obd_app_request_dtc_clear( &s_obd );
                    LOGI( TAG, "btn: dtc clear requested" );
                }
                else
                {
                    /* held on other page, no action */
                }
                break;

            case BTN_EVENT_RELEASED:
            case BTN_EVENT_NONE:
                break;

            default:
                break;
        }

        obd_app_tick( &s_obd );

        switch( obd_app_get_state( &s_obd ) )
        {
            case OBD_STATE_POLLING:
                display_update_or_halt( obd_app_get_live_data( &s_obd ),
                                        obd_app_get_dtcs( &s_obd ) );
                break;

            case OBD_STATE_ERROR:
                display_splash_or_halt( "  ECU Error     ",
                                        " Press to retry " );

                if( bsp_button_is_pressed() )
                {
                    display_splash_or_halt( " OBD Scan Tool  ",
                                            " Reconnecting.. " );
                    result = obd_app_connect( &s_obd );

                    if( RES_IS_OK( result ) )
                    {
                        LOGI( TAG, "OBD reconnected" );
                        display_app_invalidate();
                    }
                    else
                    {
                        LOGE( TAG, "OBD reconnect failed" );
                    }
                }
                break;

            case OBD_STATE_DISCONNECTED:
            case OBD_STATE_CONNECTING:
            case OBD_STATE_DISCOVERING:
            case OBD_STATE_DTC_READ:
            case OBD_STATE_DTC_CLEAR:
                /* splash already shown — no action */
                break;

            default:
                break;
        }

        /* LED blink while polling */
        if( ( now_ms - s_last_blink_ms ) >= 1000U )
        {
            s_last_blink_ms = now_ms;

            if( obd_app_get_state( &s_obd ) == OBD_STATE_POLLING )
            {
                bsp_led_green_toggle();
            }
        }
    }

    /* unreachable */
    return 0;
}
