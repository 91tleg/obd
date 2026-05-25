/**
 * @file    cmd.h
 * @brief   OBD-II service request builder
 *
 *          Builds raw OBD-II request payloads.
 */

#ifndef DRIVERS_PROTOCOL_OBD_CMD_H
#define DRIVERS_PROTOCOL_OBD_CMD_H

#include <stdint.h>

/* Service modes */
#define OBD_MODE_CURRENT_DATA    ( 0x01U )  /* live sensor data             */
#define OBD_MODE_FREEZE_FRAME    ( 0x02U )  /* freeze frame data            */
#define OBD_MODE_READ_DTC        ( 0x03U )  /* read stored DTCs             */
#define OBD_MODE_CLEAR_DTC       ( 0x04U )  /* clear DTCs                   */
#define OBD_MODE_TEST_RESULTS    ( 0x06U )  /* O2 / monitor test results    */
#define OBD_MODE_PENDING_DTC     ( 0x07U )  /* read pending DTCs            */
#define OBD_MODE_VEHICLE_INFO    ( 0x09U )  /* VIN, calibration IDs, etc.   */

/* Mode 01 PID */
#define OBD_PID_SUPPORTED_1      ( 0x00U )  /* supported PIDs 01-20 bitmask */
#define OBD_PID_ENGINE_LOAD      ( 0x04U )
#define OBD_PID_COOLANT_TEMP     ( 0x05U )
#define OBD_PID_SHORT_TERM_FUEL  ( 0x06U )
#define OBD_PID_LONG_TERM_FUEL   ( 0x07U )
#define OBD_PID_INTAKE_PRESSURE  ( 0x0BU )
#define OBD_PID_RPM              ( 0x0CU )
#define OBD_PID_VEHICLE_SPEED    ( 0x0DU )
#define OBD_PID_TIMING_ADVANCE   ( 0x0EU )
#define OBD_PID_INTAKE_TEMP      ( 0x0FU )
#define OBD_PID_MAF              ( 0x10U )
#define OBD_PID_THROTTLE         ( 0x11U )
#define OBD_PID_O2_B1S1          ( 0x14U )
#define OBD_PID_RUNTIME          ( 0x1FU )
#define OBD_PID_SUPPORTED_2      ( 0x20U )  /* supported PIDs 21-40 bitmask */
#define OBD_PID_DISTANCE_MIL     ( 0x21U )
#define OBD_PID_FUEL_LEVEL       ( 0x2FU )
#define OBD_PID_BAROMETRIC       ( 0x33U )
#define OBD_PID_SUPPORTED_3      ( 0x40U )  /* supported PIDs 41-60 bitmask */
#define OBD_PID_CONTROL_MODULE   ( 0x42U )
#define OBD_PID_AMBIENT_TEMP     ( 0x46U )
#define OBD_PID_OIL_TEMP         ( 0x5CU )
#define OBD_PID_FUEL_RATE        ( 0x5EU )
#define OBD_PID_SUPPORTED_4      ( 0x60U )  /* supported PIDs 61-80 bitmask */

/* Mode 09 PIDs */
#define OBD_PID_VIN              ( 0x02U )

#define OBD_CMD_MAX_LEN          ( 7U )     /* max payload before CAN TP    */

/**
 * @brief  Build a Mode 01 (current data) request for a single PID.
 *
 * @param  pid      OBD-II PID byte.
 * @param  buf      Output buffer, at least 2 bytes.
 * @return Payload length (always 2).
 */
uint8_t obd_cmd_current_data( uint8_t pid, uint8_t * buf );

/**
 * @brief  Build a Mode 03 (read stored DTCs) request.
 *
 * @param  buf  Output buffer, at least 1 byte.
 * @return Payload length (always 1).
 */
uint8_t obd_cmd_read_dtc( uint8_t * buf );

/**
 * @brief  Build a Mode 04 (clear DTCs) request.
 *
 * @param  buf  Output buffer, at least 1 byte.
 * @return Payload length (always 1).
 */
uint8_t obd_cmd_clear_dtc( uint8_t * buf );

/**
 * @brief  Build a Mode 07 (pending DTCs) request.
 */
uint8_t obd_cmd_pending_dtc( uint8_t * buf );

/**
 * @brief  Build a Mode 09 (vehicle info) request.
 *
 * @param  pid  Info type PID (e.g. OBD_PID_VIN = 0x02).
 */
uint8_t obd_cmd_vehicle_info( uint8_t pid, uint8_t * buf );

#endif /* DRIVERS_PROTOCOL_OBD_CMD_H */
