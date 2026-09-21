/**
 * @file ecu_sim.h
 * @brief Software ECU simulator for OBD-II SIL tests.
 *
 * Sits behind can_send(): every frame the code under test transmits is fed to
 * ecu_sim_process(), which behaves like an ISO 15765-2 ECU:
 *   - answers single-frame requests (functional 0x7DF / physical 0x7E0),
 *   - segments long responses: sends a First Frame, then releases the
 *     Consecutive Frames only once the tester answers with Flow Control CTS,
 *   - stays silent for anything it has no entry for (=> tester times out).
 *
 * Fault injection lets tests exercise the tester's error handling.
 */

#ifndef TEST_ECU_SIM_H
#define TEST_ECU_SIM_H

#include <stdint.h>
#include <stdbool.h>
#include "can_stub.h"

#define ECU_SIM_MAX_ENTRIES  (64U)
#define ECU_SIM_MAX_DATA     (120U)
#define ECU_SIM_DEFAULT_RESP_ID  (0x7E8U)

typedef enum
{
    ECU_SIM_FAULT_NONE = 0,
    ECU_SIM_FAULT_WRONG_PID_ECHO,   /* Mode 01 reply echoes the wrong PID     */
    ECU_SIM_FAULT_BAD_SN,           /* 2nd Consecutive Frame has a bad SN     */
    ECU_SIM_FAULT_DROP_CFS,         /* First Frame sent, CFs never arrive     */
    ECU_SIM_FAULT_WRONG_ID          /* reply uses an ID outside 0x7E8-0x7EF   */
} ecu_sim_fault_t;

/** Reset the simulator (entries, faults, counters). Leaves it disabled. */
void ecu_sim_reset(void);

/** Enable or disable automatic responses. Disabled = ECU is silent. */
void ecu_sim_enable(bool enable);

/** Select a fault to inject into subsequent responses. */
void ecu_sim_set_fault(ecu_sim_fault_t fault);

/** Set the CAN ID used for responses (default 0x7E8). */
void ecu_sim_set_response_id(uint32_t id);

/**
 * Register a PID-addressed response (Modes 01/02/09).
 * Positive reply payload: [mode+0x40][pid][data...]. Re-registering the same
 * mode/pid replaces the earlier entry.
 */
void ecu_sim_register_pid(uint8_t mode,
                          uint8_t pid,
                          uint8_t const *data,
                          uint8_t len);

/**
 * Register a mode-only response (Modes 03/04/07, no PID echo).
 * Positive reply payload: [mode+0x40][data...]. data may be NULL if len is 0.
 */
void ecu_sim_register_mode(uint8_t mode,
                           uint8_t const *data,
                           uint8_t len);

/**
 * Register a negative response 7F <mode> <nrc> for a mode/pid request.
 */
void ecu_sim_register_negative(uint8_t mode, uint8_t pid);

/**
 * Queue a raw single frame with arbitrary payload, bypassing the simulator's
 * logic. For malformed-response tests. payload_len <= 7.
 */
void ecu_sim_inject_sf(uint32_t id, uint8_t const *payload, uint8_t payload_len);

/** Queue a raw frame with arbitrary bytes (e.g. an unknown PCI type). */
void ecu_sim_inject_raw(uint32_t id, uint8_t const *bytes, uint8_t len);

/** Feed a transmitted frame to the simulator. Register with stub_set_on_send. */
void ecu_sim_process(can_frame_t const *frame);

/** Number of requests the simulator has decoded since reset. */
uint32_t ecu_sim_request_count(void);

/** Number of Flow Control CTS frames received from the tester since reset. */
uint32_t ecu_sim_fc_count(void);

#endif /* TEST_ECU_SIM_H */
