#ifndef PUS_H
#define PUS_H

#include <stdint.h>

// ── PUS packet structures ─────────────────────────────────────────────────────

typedef struct {
    uint8_t  version;
    uint8_t  type;      // 0 = TM, 1 = TC
    uint8_t  service;
    uint8_t  subtype;
    uint16_t length;    // data payload length in bytes
} pus_header_t;

typedef struct {
    pus_header_t header;
    uint8_t      data[32];
} pus_packet_t;

// ── Satellite state (shared between OBC and ground) ──────────────────────────

typedef enum {
    SYS_IDLE,       // Standby, no mission activity
    SYS_ACTIVE,     // Acquiring sensor data onboard
    SYS_DOWNLINK    // Transmitting data to ground station
} system_state_t;

// ── PUS-C standard service numbers ───────────────────────────────────────────

#define PUS_SVC_VERIFICATION   1   // Request Verification
#define PUS_SVC_HK             3   // Housekeeping
#define PUS_SVC_TEST          17   // Connection Test

// Custom mission services (128–255 per ECSS-E-ST-70-41)
#define PUS_SVC_MODE_CTRL    128   // Satellite mode / state control
#define PUS_SVC_SENSOR_DATA  129   // Mission sensor telemetry

// ── Service 1 — Request Verification (TM only, sent by OBC) ──────────────────
#define PUS_TM_ACCEPT_OK       1   // TC Acceptance Report – success
#define PUS_TM_ACCEPT_FAIL     2   // TC Acceptance Report – failure

// ── Service 3 — Housekeeping ─────────────────────────────────────────────────
#define PUS_TC_HK_REQUEST     27   // TC: request HK report on demand
#define PUS_TM_HK_REPORT      25   // TM: HK parameter report (12 bytes)

// ── Service 17 — Connection Test ─────────────────────────────────────────────
#define PUS_TC_ARE_YOU_ALIVE   1   // TC: perform connection test
#define PUS_TM_I_AM_ALIVE      2   // TM: connection test report

// ── Service 128 — Mode Control (custom) ──────────────────────────────────────
#define PUS_TC_ACTIVATE        1   // TC: IDLE → ACTIVE
#define PUS_TC_START_DOWNLINK  2   // TC: ACTIVE → DOWNLINK
#define PUS_TC_STOP_DOWNLINK   3   // TC: DOWNLINK → ACTIVE
#define PUS_TC_DEACTIVATE      4   // TC: ACTIVE → IDLE

// ── Service 129 — Sensor Data (custom) ───────────────────────────────────────
#define PUS_TM_SENSOR_READING  1   // TM: single sensor measurement (2 bytes, LE int16)

#endif
