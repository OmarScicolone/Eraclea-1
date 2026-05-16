#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "pus.h"   // for system_state_t

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void system_init(void);

// ── State transitions ─────────────────────────────────────────────────────────

void system_activate(void);        // IDLE → ACTIVE
void system_deactivate(void);      // ACTIVE → IDLE
void system_start_downlink(void);  // ACTIVE → DOWNLINK
void system_stop_downlink(void);   // DOWNLINK → ACTIVE

system_state_t system_get_state(void);
uint32_t       system_get_uptime(void);

#define HK_REPORT_LEN 12
void system_build_hk_report(uint8_t* buf);  // fills HK_REPORT_LEN bytes

// ── OBC tasks (RTOS-like threads) ─────────────────────────────────────────────

void* sensor_task(void* arg);
void* processing_task(void* arg);
void* health_task(void* arg);

#endif
