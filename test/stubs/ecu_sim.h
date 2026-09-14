/**
 * @file ecu_sim.h
 * @brief Software ECU simulator for OBD-II SIL tests.
 *
 * Intercepts CAN requests and queues simulated ECU responses.
 */

#ifndef TEST_ECU_SIM_H
#define TEST_ECU_SIM_H

#include <stdint.h>
#include <stdbool.h>
#include "can_stub.h"

#define ECU_SIM_MAX_PIDS  (64U)

/** Reset the simulator and disable auto-processing. */
void ecu_sim_reset(void);

/**
 * Register a simulated PID response.
 *
 * @param mode OBD mode.
 * @param pid  PID.
 * @param data Raw response bytes.
 * @param len  Response length.
 */
void ecu_sim_register_pid(uint8_t mode,
                          uint8_t pid,
                          uint8_t const *data,
                          uint8_t len);

/** Enable or disable automatic response processing. */
void ecu_sim_enable(bool enable);

/** Process a transmitted CAN frame and queue an ECU response. */
void ecu_sim_process(can_frame_t const *frame);

/** Register a PID to return NRC 0x31 (request out of range). */
void ecu_sim_register_negative(uint8_t mode, uint8_t pid);

#endif /* TEST_ECU_SIM_H */
