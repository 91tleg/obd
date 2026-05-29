/**
 * @file    kwp2000_timing.h
 * @brief   ISO 14230 timing
 */

#ifndef DRIVERS_OBD_KWP2000_TIMING_H
#define DRIVERS_OBD_KWP2000_TIMING_H

/** Maximum inter-byte gap within an ECU response */
#define KWP_P1_MAX_MS       ( 20U )

/** Minimum time from end of tester request to start of ECU response */
#define KWP_P2_MIN_MS       ( 25U )

/** Maximum time from end of tester request to start of ECU response */
#define KWP_P2_MAX_MS       ( 50U )

/** Minimum time from end of ECU response to start of next request */
#define KWP_P3_MIN_MS       ( 55U )

/** Maximum time between requests before ECU drops session */
#define KWP_P3_MAX_MS       ( 5000U )

/** Minimum inter-byte gap within a tester request */
#define KWP_P4_MIN_MS       ( 5U )

/** Maximum inter-byte gap within a tester request */
#define KWP_P4_MAX_MS       ( 20U )

/** Bus idle time before fast init pulse */
#define KWP_W0_MIN_MS       ( 2U )

/** Duration of fast init low pulse */
#define KWP_W_LOW_MS        ( 25U )

/** Duration of fast init high pulse */
#define KWP_W_HIGH_MS       ( 25U )

/** 5-baud bit period — 200ms per bit at 5 baud */
#define KWP_5BAUD_BIT_MS    ( 200U )

/** Bus idle before 5-baud address byte */
#define KWP_W5_MIN_MS       ( 300U )

/** Time to wait for sync byte after 5-baud address */
#define KWP_SYNC_TIMEOUT_MS ( 500U )

/** Time to wait for keyword bytes after sync */
#define KWP_KW_TIMEOUT_MS   ( 30U )

/** Time between keyword 2 reception and tester response */
#define KWP_W4_MIN_MS       ( 25U )
#define KWP_W4_MAX_MS       ( 50U )

#define KWP_KEEPALIVE_MS    ( KWP_P3_MAX_MS - 500U )

#endif /* DRIVERS_OBD_KWP2000_TIMING_H */
