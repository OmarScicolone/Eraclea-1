#include "ground_sim.h"
#include "tc_handler.h"
#include <stdio.h>
#include <time.h>

static int tc_counter = 0;

void simulate_ground_tc(void)
{
    pus_packet_t pkt;

    // Cycle through different commands
    switch (tc_counter++ % 6) {
        case 0:
            // Switch ON
            pkt.header.version = 1;
            pkt.header.type = 1; // TC
            pkt.header.service = 1;
            pkt.header.subtype = 1;
            pkt.header.length = 0;
            printf("[GROUND] Sending SWITCH ON command\n");
            break;

        case 1:
            // Enable Data
            pkt.header.version = 1;
            pkt.header.type = 1;
            pkt.header.service = 1;
            pkt.header.subtype = 2;
            pkt.header.length = 0;
            printf("[GROUND] Sending ENABLE DATA command\n");
            break;

        case 2:
            // Test command
            pkt.header.version = 1;
            pkt.header.type = 1; // TC
            pkt.header.service = 17;
            pkt.header.subtype = 1;
            pkt.header.length = 0;
            printf("[GROUND] Sending TEST command\n");
            break;

        case 3:
            // Housekeeping request
            pkt.header.version = 1;
            pkt.header.type = 1;
            pkt.header.service = 3;
            pkt.header.subtype = 25;
            pkt.header.length = 0;
            printf("[GROUND] Sending HK request\n");
            break;

        case 4:
            // Shutdown
            pkt.header.version = 1;
            pkt.header.type = 1;
            pkt.header.service = 1;
            pkt.header.subtype = 3;
            pkt.header.length = 0;
            printf("[GROUND] Sending SHUTDOWN command\n");
            break;

        default:
            // Unknown command (error test)
            pkt.header.version = 1;
            pkt.header.type = 1;
            pkt.header.service = 99;
            pkt.header.subtype = 99;
            pkt.header.length = 0;
            printf("[GROUND] Sending UNKNOWN command\n");
            break;
    }

    handle_tc(&pkt);
}

void print_tm_output(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len)
{
    printf("[GROUND OUTPUT] TM: Service=%d, Subtype=%d, Len=%d\n", service, subtype, len);
    
    if (data && len > 0) {
        printf("  Data: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
    }
}
