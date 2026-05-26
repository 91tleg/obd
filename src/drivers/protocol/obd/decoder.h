/**
 * @file    decoder.h
 * @brief   OBD-II response payload decoder
 *
 * All Mode 01 PID scaling formulas are defined in ISO 15031-5 / SAE J1979.
 * Identical across all compliant vehicles.
 *
 * Formula notation from the spec:
 *   A = first data byte, B = second, C = third, D = fourth.
 */

#ifndef OBD_DECODER_H
#define OBD_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"

/**
 * @brief  Decode a Mode 01 current-data response.
 *
 * @param  pid        The PID that was requested.
 * @param  payload    Response bytes after the mode+PID echo (A, B, C, D...).
 * @param  len        Length of @p payload.
 * @param  param_out  Decoded parameter.
 * @param  timestamp  Tick at time of reception.
 * @return RES_OK, RES_ERR_UNSUPPORTED (PID not in decoder),
 *         or RES_ERR_INVALID_ARG.
 */
result_t obd_decode_pid( uint8_t pid,
                         uint8_t * payload,
                         uint8_t len,
                         live_data_param_t * param_out,
                         uint32_t timestamp );

/**
 * @brief  Decode a Mode 03 or Mode 07 DTC response.
 *
 * @param  payload   Response bytes after the mode byte echo.
 * @param  len       Length of @p payload.
 * @param  pending   true if these are pending DTCs (Mode 07).
 * @param  dtc_out   Output list; entries appended up to capacity.
 * @return RES_OK or RES_ERR_INVALID_ARG.
 */
result_t obd_decode_dtcs( uint8_t * payload,
                          uint32_t len,
                          bool pending,
                          dtc_list_t * dtc_out );

/**
 * @brief  Decode Mode 01 PID 0x00 / 0x20 / 0x40 / 0x60 supported-PID
 *         bitmask into an array of supported PID bytes.
 *
 * @param  payload       4-byte bitmask from the ECU.
 * @param  base_pid      The support PID that was queried (0x00, 0x20 etc.).
 * @param  pids_out      Output array.
 * @param  max_pids      Capacity of @p pids_out.
 * @param  count_out     Number of supported PIDs found.
 */
result_t obd_decode_supported_pids( uint8_t * payload,
                                    uint8_t base_pid,
                                    uint8_t * pids_out,
                                    uint32_t max_pids,
                                    uint32_t * count_out );

#endif /* OBD_DECODER_H */
