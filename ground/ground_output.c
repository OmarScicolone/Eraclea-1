#include "ground_output.h"
#include "pus.h"
#include <stdio.h>

static const char* tc_describe(uint8_t svc, uint8_t sub)
{
    if (svc == PUS_SVC_TEST      && sub == PUS_TC_ARE_YOU_ALIVE)   return "Connection Test: Are-You-Alive";
    if (svc == PUS_SVC_HK        && sub == PUS_TC_HK_REQUEST)      return "Housekeeping: Report Request";
    if (svc == PUS_SVC_MODE_CTRL && sub == PUS_TC_ACTIVATE)        return "Mode Control: Activate";
    if (svc == PUS_SVC_MODE_CTRL && sub == PUS_TC_START_DOWNLINK)  return "Mode Control: Start Downlink";
    if (svc == PUS_SVC_MODE_CTRL && sub == PUS_TC_STOP_DOWNLINK)   return "Mode Control: Stop Downlink";
    if (svc == PUS_SVC_MODE_CTRL && sub == PUS_TC_DEACTIVATE)      return "Mode Control: Deactivate";
    return "Unknown TC";
}

void print_tm_output(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len)
{
    // ── Service 1 — Request Verification ─────────────────────────────────────
    if (service == PUS_SVC_VERIFICATION) {
        const char* verdict = (subtype == PUS_TM_ACCEPT_OK) ? "Accepted" : "Rejected";
        if (data && len >= 2) {
            uint8_t tc_svc = data[0];
            uint8_t tc_sub = data[1];
            printf("\nTM [%d,%d] <- Verification: TC [%d,%d] %s  (%s)\n",
                   service, subtype, tc_svc, tc_sub, verdict, tc_describe(tc_svc, tc_sub));
        } else {
            printf("\nTM [%d,%d] <- Verification: %s\n", service, subtype, verdict);
        }
        return;
    }

    // ── Service 3 — HK Report ────────────────────────────────────────────────
    if (service == PUS_SVC_HK && subtype == PUS_TM_HK_REPORT) {
        printf("\nTM [%d,%d] <- Housekeeping Report\n", service, subtype);
        if (data && len >= 12) {
            uint32_t uptime    = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16)
                               | ((uint32_t)data[2] << 8)  |  (uint32_t)data[3];
            uint16_t sensor_ok  = ((uint16_t)data[4] << 8) | data[5];
            uint16_t sensor_err = ((uint16_t)data[6] << 8) | data[7];
            uint8_t  buf_ovf    = data[8];
            uint8_t  buf_occ    = data[9];
            uint16_t tm_sent    = ((uint16_t)data[10] << 8) | data[11];
            printf("    Uptime:              %u s\n",    uptime);
            printf("    Sensor OK / Error:   %u / %u\n", sensor_ok, sensor_err);
            printf("    Buffer fill / OVF:   %u / %u\n", buf_occ,   buf_ovf);
            printf("    TM sent:             %u\n",      tm_sent);
        }
        return;
    }

    // ── Service 17 — Connection Test ─────────────────────────────────────────
    if (service == PUS_SVC_TEST && subtype == PUS_TM_I_AM_ALIVE) {
        printf("\nTM [%d,%d] <- Connection Test: OBC is alive\n", service, subtype);
        return;
    }

    // ── Service 129 — Sensor Data ─────────────────────────────────────────────
    if (service == PUS_SVC_SENSOR_DATA && subtype == PUS_TM_SENSOR_READING) {
        if (data && len >= 2) {
            int16_t value = (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
            printf("\nTM [%d,%d] <- Sensor Data (value=%d)\n", service, subtype, value);
        }
        return;
    }

    // ── Unknown / raw fallback ────────────────────────────────────────────────
    printf("\nTM [%d,%d] <- Unknown (len=%d)", service, subtype, len);
    if (data && len > 0) {
        printf("  [");
        for (uint16_t i = 0; i < len; i++)
            printf(" %02X", data[i]);
        printf(" ]");
    }
    printf("\n");
}
