/**
 * @file can_stub.h
 * @brief CAN HAL stub for SIL testing.
 */

#ifndef TEST_CAN_STUB_H
#define TEST_CAN_STUB_H

#include <stdint.h>
#include <stdbool.h>
#include "lib/core/result.h"

#define CAN_FRAME_DATA_LEN     (8U)
#define CAN_FRAME_FD_DATA_LEN  (64U)
#define CAN_FD_DLC_64          (15U)

typedef struct
{
    uint32_t id;
    uint8_t  data[CAN_FRAME_FD_DATA_LEN];
    uint8_t  dlc;
    bool     fd;
    bool     brs;
} can_frame_t;

/* Opaque peripheral type. */
typedef struct { uint32_t dummy; } FDCAN_GlobalTypeDef;

#define STUB_MAX_FRAMES  (128U)

/** Reset all stub state. */
void stub_reset(void);

/** Queue a frame for the next can_recv call. */
void stub_queue_recv(can_frame_t const *frame);

/** Set the error returned by the next can_recv call. */
void stub_set_recv_error(result_t err);

/** Return the number of captured can_send frames. */
uint32_t stub_sent_count(void);

/** Return a captured can_send frame by index. */
can_frame_t const *stub_sent(uint32_t index);

/** Return the number of frames in the receive queue. */
uint32_t stub_recv_remaining(void);

/** Register a callback invoked by can_send. */
void stub_set_on_send(void (*cb)(can_frame_t const *));

#endif /* TEST_CAN_STUB_H */
