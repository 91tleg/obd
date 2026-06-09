/**
 * @file    can_tp.h
 * @brief   ISO 15765-2 CAN transport layer (CAN TP)
 *
 *          Handles segmentation and reassembly of OBD-II payloads over
 *          classical CAN frames. Caller supplies raw frame send/recv via
 *          the hardware contract below.
 *
 *          OBD-II CAN IDs:
 *            Functional request:  0x7DF  (broadcast to all ECUs)
 *            ECU response range:  0x7E8 - 0x7EF
 *
 *          Frame types (ISO 15765-2):
 *            Single Frame  (SF):  PCI byte 0x0n, n = data length
 *            First Frame   (FF):  PCI bytes 0x1n nn, multi-frame start
 *            Consecutive   (CF):  PCI byte 0x2n, n = sequence index
 *            Flow Control  (FC):  PCI byte 0x3n, sent by receiver
 */

#ifndef DRIVERS_OBD_CAN_TP_H
#define DRIVERS_OBD_CAN_TP_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"

#define CAN_TP_OBD_REQ_ID       ( 0x7DFU )  /* functional broadcast         */
#define CAN_TP_OBD_RESP_BASE    ( 0x7E8U )  /* first ECU response ID        */
#define CAN_TP_OBD_RESP_MAX     ( 0x7EFU )  /* last ECU response ID         */

#define CAN_TP_MAX_PAYLOAD  ( 256U )   /* ISO 15765-2 max for classical CAN */

typedef struct
{
    uint8_t  rx_buf[ CAN_TP_MAX_PAYLOAD ];
    uint32_t rx_len;
    uint8_t  block_size;
    uint8_t  st_min_ms;
    uint8_t  sn_expected;   /* expected consecutive frame sequence number */
    bool     use_fd;        /* set true to enable CAN FD 64-byte mode     */
} can_tp_ctx_t;

/**
 * Initialize the transport context.
 */
result_t can_tp_init( can_tp_ctx_t * ctx );

/**
 * Send an OBD-II payload — handles SF or FF/CF segmentation automatically.
 *
 * @param ctx       Transport context.
 * @param req_id    CAN ID to send to (use CAN_TP_OBD_REQ_ID for broadcast).
 * @param payload   Raw OBD payload (mode + PID bytes, before framing).
 * @param len       Payload length in bytes.
 * @return RES_OK, RES_ERR_INVALID_ARG, or RES_ERR_BUSY.
 */
result_t can_tp_send( can_tp_ctx_t * ctx,
                      uint32_t req_id,
                      uint8_t const * payload,
                      uint32_t len );

/**
 * Receive an OBD-II response — handles SF or FF/CF reassembly automatically.
 * Sends flow control frames as required by ISO 15765-2.
 *
 * @param ctx        Transport context.
 * @param buf        Output buffer for reassembled payload.
 * @param buf_size   Capacity of buf.
 * @param out_len    Number of bytes written to buf.
 * @param timeout_ms Maximum time to wait for first frame.
 * @return RES_OK, RES_ERR_INVALID_ARG, RES_ERR_TIMEOUT, or RES_ERR_BUS.
 */
result_t can_tp_recv( can_tp_ctx_t * ctx,
                      uint8_t * buf,
                      uint32_t buf_size,
                      uint32_t * out_len,
                      uint32_t timeout_ms );

#endif /* DRIVERS_OBD_CAN_TP_H */
