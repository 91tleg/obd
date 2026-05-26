/**
 * @file    obd_types.h
 * @brief   Shared OBD-II data types
 */

#ifndef DRIVERS_OBD_TYPES_H
#define DRIVERS_OBD_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    OBD_PARAM_ENGINE_LOAD       = 0U,
    OBD_PARAM_COOLANT_TEMP      = 1U,
    OBD_PARAM_SHORT_TERM_FUEL   = 2U,
    OBD_PARAM_LONG_TERM_FUEL    = 3U,
    OBD_PARAM_INTAKE_PRESSURE   = 4U,
    OBD_PARAM_RPM               = 5U,
    OBD_PARAM_VEHICLE_SPEED     = 6U,
    OBD_PARAM_TIMING_ADVANCE    = 7U,
    OBD_PARAM_INTAKE_TEMP       = 8U,
    OBD_PARAM_MAF               = 9U,
    OBD_PARAM_THROTTLE          = 10U,
    OBD_PARAM_RUNTIME           = 11U,
    OBD_PARAM_DISTANCE_MIL      = 12U,
    OBD_PARAM_FUEL_LEVEL        = 13U,
    OBD_PARAM_BAROMETRIC        = 14U,
    OBD_PARAM_CONTROL_MODULE_V  = 15U,
    OBD_PARAM_AMBIENT_TEMP      = 16U,
    OBD_PARAM_OIL_TEMP          = 17U,
    OBD_PARAM_FUEL_RATE         = 18U,
    OBD_PARAM_COUNT             = 19U,
} obd_param_id_t;

typedef struct
{
    obd_param_id_t id;          /* which parameter                         */
    float          value;       /* decoded engineering value               */
    uint32_t       timestamp;   /* tick at time of reception (ms)          */
    bool           valid;       /* false until first successful decode     */
} live_data_param_t;

#define LIVE_DATA_MAX_PARAMS  ( ( uint32_t )OBD_PARAM_COUNT )

typedef struct
{
    live_data_param_t params[ LIVE_DATA_MAX_PARAMS ];
    uint32_t          count;    /* number of valid entries                 */
} live_data_t;

typedef enum
{
    DTC_TYPE_POWERTRAIN = 0U,   /* P codes */
    DTC_TYPE_CHASSIS    = 1U,   /* C codes */
    DTC_TYPE_BODY       = 2U,   /* B codes */
    DTC_TYPE_NETWORK    = 3U,   /* U codes */
} dtc_type_t;

typedef struct
{
    uint16_t   code;      /* raw 16-bit code as received from ECU          */
    dtc_type_t type;      /* powertrain / chassis / body / network         */
    bool       pending;   /* true = Mode 07 (pending), false = Mode 03    */
} dtc_entry_t;

#define DTC_LIST_MAX  ( 32U )

typedef struct
{
    dtc_entry_t codes[ DTC_LIST_MAX ];
    uint32_t    count;
} dtc_list_t;

#endif /* DRIVERS_OBD_TYPES_H */
