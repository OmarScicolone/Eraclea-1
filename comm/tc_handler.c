#include "tc_handler.h"
#include "tm_manager.h"
#include "system.h"
#include "sensor.h"
#include "buffer.h"
#include <stdio.h>

static void ack(uint8_t svc, uint8_t sub) {
    uint8_t ref[2] = { svc, sub };
    send_tm(PUS_SVC_VERIFICATION, PUS_TM_ACCEPT_OK, ref, 2);
}
static void nack(uint8_t svc, uint8_t sub) {
    uint8_t ref[2] = { svc, sub };
    send_tm(PUS_SVC_VERIFICATION, PUS_TM_ACCEPT_FAIL, ref, 2);
}

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

void handle_tc(pus_packet_t* pkt)
{
    if (!pkt) return;

    uint8_t svc = pkt->header.service;
    uint8_t sub = pkt->header.subtype;

    printf("\nTC [%d,%d] <- %s\n", svc, sub, tc_describe(svc, sub));

    switch (svc) {

        // ── Service 17 — Connection Test ─────────────────────────────────────
        case PUS_SVC_TEST:
            if (sub == PUS_TC_ARE_YOU_ALIVE) {
                send_tm(PUS_SVC_TEST, PUS_TM_I_AM_ALIVE, NULL, 0);
            } else {
                nack(svc, sub);
            }
            break;

        // ── Service 3 — Housekeeping (on-demand report) ───────────────────────
        case PUS_SVC_HK:
            if (sub == PUS_TC_HK_REQUEST) {
                uint8_t hk[12];
                uint8_t len = system_build_hk_report(hk);
                send_tm(PUS_SVC_HK, PUS_TM_HK_REPORT, hk, len);
                ack(svc, sub);
            } else {
                nack(svc, sub);
            }
            break;

        // ── Service 128 — Mode Control ────────────────────────────────────────
        case PUS_SVC_MODE_CTRL:
            switch (sub) {
                case PUS_TC_ACTIVATE:
                    system_activate();
                    ack(svc, sub);
                    break;
                case PUS_TC_START_DOWNLINK:
                    system_start_downlink();
                    ack(svc, sub);
                    break;
                case PUS_TC_STOP_DOWNLINK:
                    system_stop_downlink();
                    ack(svc, sub);
                    break;
                case PUS_TC_DEACTIVATE:
                    system_deactivate();
                    ack(svc, sub);
                    break;
                default:
                    printf("  Unknown mode control subtype: %d\n", sub);
                    nack(svc, sub);
                    break;
            }
            break;

        default:
            printf("  Unknown service %d — TC rejected\n", svc);
            nack(svc, sub);
            break;
    }
}
