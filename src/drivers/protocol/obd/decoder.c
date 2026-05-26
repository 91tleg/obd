/**
 * @file    decoder.c
 * @brief   OBD-II response payload decoder
 */

#include "drivers/protocol/obd/decoder.h"
#include "drivers/protocol/obd/cmd.h"
#include "drivers/protocol/obd/obd_types.h"

/* Big endian */
static uint16_t u16_be( uint8_t const * p )
{
    return ( uint16_t )( ( ( uint16_t )p[ 0U ] << 8U ) | ( uint16_t )p[ 1U ] );
}

static result_t decode_pid_value( uint8_t pid,
                                  uint8_t const * payload,
                                  uint8_t len,
                                  float * value_out,
                                  uint32_t * param_id_out )
{
    result_t result = RES_ERR_UNSUPPORTED;

    switch( pid )
    {
        case OBD_PID_ENGINE_LOAD:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ] / 2.55F;
                *param_id_out = OBD_PARAM_ENGINE_LOAD;
                result        = RES_OK;
            }
            break;

        case OBD_PID_COOLANT_TEMP:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ] - 40.0F;
                *param_id_out = OBD_PARAM_COOLANT_TEMP;
                result        = RES_OK;
            }
            break;

        case OBD_PID_SHORT_TERM_FUEL:
            if( len >= 1U )
            {
                *value_out    = ( ( float )payload[ 0U ] / 1.28F ) - 100.0F;
                *param_id_out = OBD_PARAM_SHORT_TERM_FUEL;
                result        = RES_OK;
            }
            break;

        case OBD_PID_LONG_TERM_FUEL:
            if( len >= 1U )
            {
                *value_out    = ( ( float )payload[ 0U ] / 1.28F ) - 100.0F;
                *param_id_out = OBD_PARAM_LONG_TERM_FUEL;
                result        = RES_OK;
            }
            break;

        case OBD_PID_INTAKE_PRESSURE:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ];
                *param_id_out = OBD_PARAM_INTAKE_PRESSURE;
                result        = RES_OK;
            }
            break;

        case OBD_PID_RPM:
            if( len >= 2U )
            {
                *value_out    = ( float )u16_be( payload ) / 4.0F;
                *param_id_out = OBD_PARAM_RPM;
                result        = RES_OK;
            }
            break;

        case OBD_PID_VEHICLE_SPEED:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ];
                *param_id_out = OBD_PARAM_VEHICLE_SPEED;
                result        = RES_OK;
            }
            break;

        case OBD_PID_TIMING_ADVANCE:
            if( len >= 1U )
            {
                *value_out    = ( ( float )payload[ 0U ] / 2.0F ) - 64.0F;
                *param_id_out = OBD_PARAM_TIMING_ADVANCE;
                result        = RES_OK;
            }
            break;

        case OBD_PID_INTAKE_TEMP:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ] - 40.0F;
                *param_id_out = OBD_PARAM_INTAKE_TEMP;
                result        = RES_OK;
            }
            break;

        case OBD_PID_MAF:
            if( len >= 2U )
            {
                *value_out    = ( float )u16_be( payload ) / 100.0F;
                *param_id_out = OBD_PARAM_MAF;
                result        = RES_OK;
            }
            break;

        case OBD_PID_THROTTLE:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ] / 2.55F;
                *param_id_out = OBD_PARAM_THROTTLE;
                result        = RES_OK;
            }
            break;

        case OBD_PID_RUNTIME:
            if( len >= 2U )
            {
                *value_out    = ( float )u16_be( payload );
                *param_id_out = OBD_PARAM_RUNTIME;
                result        = RES_OK;
            }
            break;

        case OBD_PID_DISTANCE_MIL:
            if( len >= 2U )
            {
                *value_out    = ( float )u16_be( payload );
                *param_id_out = OBD_PARAM_DISTANCE_MIL;
                result        = RES_OK;
            }
            break;

        case OBD_PID_FUEL_LEVEL:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ] / 2.55F;
                *param_id_out = OBD_PARAM_FUEL_LEVEL;
                result        = RES_OK;
            }
            break;

        case OBD_PID_BAROMETRIC:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ];
                *param_id_out = OBD_PARAM_BAROMETRIC;
                result        = RES_OK;
            }
            break;

        case OBD_PID_CONTROL_MODULE:
            if( len >= 2U )
            {
                *value_out    = ( float )u16_be( payload ) / 1000.0F;
                *param_id_out = OBD_PARAM_CONTROL_MODULE_V;
                result        = RES_OK;
            }
            break;

        case OBD_PID_AMBIENT_TEMP:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ] - 40.0F;
                *param_id_out = OBD_PARAM_AMBIENT_TEMP;
                result        = RES_OK;
            }
            break;

        case OBD_PID_OIL_TEMP:
            if( len >= 1U )
            {
                *value_out    = ( float )payload[ 0U ] - 40.0F;
                *param_id_out = OBD_PARAM_OIL_TEMP;
                result        = RES_OK;
            }
            break;

        case OBD_PID_FUEL_RATE:
            if( len >= 2U )
            {
                *value_out    = ( float )u16_be( payload ) / 20.0F;
                *param_id_out = OBD_PARAM_FUEL_RATE;
                result        = RES_OK;
            }
            break;

        default:
            result = RES_ERR_UNSUPPORTED;
            break;
    }

    return result;
}

static dtc_type_t dtc_type_from_byte( uint8_t high_byte )
{
    dtc_type_t type;

    switch( ( high_byte >> 6U ) & 0x03U )
    {
        case 0x00U:
        case 0x01U:
            type = DTC_TYPE_POWERTRAIN;
            break;
        case 0x02U:
            type = DTC_TYPE_CHASSIS;
            break;
        case 0x03U:
            type = DTC_TYPE_BODY;
            break;
        default:
            /* unreachable — 2-bit value fully enumerated */
            type = DTC_TYPE_POWERTRAIN;
            break;
    }

    return type;
}

result_t obd_decode_pid( uint8_t pid,
                         uint8_t const * payload,
                         uint8_t len,
                         live_data_param_t * param_out,
                         uint32_t timestamp )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( payload != NULL ) && ( param_out != NULL ) && ( len > 0U ) )
    {
        float value = 0.0F;
        uint32_t param_id = 0U;

        result = decode_pid_value( pid, payload, len, &value, &param_id );

        if( RES_IS_OK( result ) )
        {
            param_out->id        = param_id;
            param_out->value     = value;
            param_out->timestamp = timestamp;
        }
    }

    return result;
}

result_t obd_decode_dtcs( uint8_t const * payload,
                          uint32_t len,
                          bool pending,
                          dtc_list_t * dtc_out )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( payload != NULL ) && ( dtc_out != NULL ) )
    {
        uint32_t dtc_count = ( len > 0U ) ? ( uint32_t )payload[ 0U ] : 0U;
        uint32_t offset = 1U;
        uint32_t i = 0U;

        dtc_out->count = 0U;

        while( ( i < dtc_count ) &&
               ( dtc_out->count < DTC_LIST_MAX ) &&
               ( ( offset + 2U ) <= len ) )
        {
            uint8_t hi = payload[ offset ];
            uint8_t lo = payload[ offset + 1U ];
            uint16_t code = ( uint16_t )( ( ( uint16_t )hi << 8U ) | lo );

            if( code != 0x0000U )
            {
                dtc_out->codes[ dtc_out->count ].code = code;
                dtc_out->codes[ dtc_out->count ].type = dtc_type_from_byte( hi );
                dtc_out->codes[ dtc_out->count ].pending = pending;
                dtc_out->count++;
            }

            offset += 2U;
            ++i;
        }

        result = RES_OK;
    }

    return result;
}

result_t obd_decode_supported_pids( uint8_t const * payload,
                                    uint8_t base_pid,
                                    uint8_t * pids_out,
                                    uint32_t max_pids,
                                    uint32_t * count_out )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( payload != NULL ) && ( pids_out != NULL ) &&
        ( count_out != NULL ) && ( max_pids > 0U ) )
    {
        uint32_t mask = ( ( uint32_t )payload[ 0U ] << 24U ) |
                        ( ( uint32_t )payload[ 1U ] << 16U ) |
                        ( ( uint32_t )payload[ 2U ] <<  8U ) |
                        ( ( uint32_t )payload[ 3U ]        );

        uint32_t bit = 0U;
        *count_out   = 0U;

        while( ( bit < 32U ) && ( *count_out < max_pids ) )
        {
            if( ( mask & ( 0x80000000UL >> bit ) ) != 0U )
            {
                uint16_t pid = ( uint16_t )base_pid + ( uint16_t )( bit + 1U );

                if( pid <= 0xFFU )
                {
                    pids_out[ *count_out ] = ( uint8_t )pid;
                    ( *count_out )++;
                }
            }

            ++bit;
        }

        result = RES_OK;
    }

    return result;
}
