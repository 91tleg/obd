/**
 * @file    cmd.c
 * @brief   OBD-II service request builder
 */

#include "drivers/protocol/obd/cmd.h"

uint8_t obd_cmd_current_data( uint8_t pid, uint8_t * buf )
{
    uint8_t len = 0U;

    if( buf != NULL )
    {
        buf[ 0U ] = OBD_MODE_CURRENT_DATA;
        buf[ 1U ] = pid;
        len = 2U;
    }

    return len;
}

uint8_t obd_cmd_read_dtc( uint8_t * buf )
{
    uint8_t len = 0U;

    if( buf != NULL )
    {
        buf[ 0U ] = OBD_MODE_READ_DTC;
        len = 1U;
    }

    return len;
}

uint8_t obd_cmd_clear_dtc( uint8_t * buf )
{
    uint8_t len = 0U;

    if( buf != NULL )
    {
        buf[ 0U ] = OBD_MODE_CLEAR_DTC;
        len = 1U;
    }

    return len;
}

uint8_t obd_cmd_pending_dtc( uint8_t * buf )
{
    uint8_t len = 0U;

    if( buf != NULL )
    {
        buf[ 0U ] = OBD_MODE_PENDING_DTC;
        len = 1U;
    }

    return len;
}

uint8_t obd_cmd_vehicle_info( uint8_t pid, uint8_t * buf )
{
    uint8_t len = 0U;

    if( buf != NULL )
    {
        buf[ 0U ] = OBD_MODE_VEHICLE_INFO;
        buf[ 1U ] = pid;
        len = 2U;
    }

    return len;
}
