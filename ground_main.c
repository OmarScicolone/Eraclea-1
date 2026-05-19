#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "link.h"
#include "pus.h"
#include "platform.h"
#include "ground_output.h"

static int            obc_fd       = -1;
static system_state_t ground_state = SYS_IDLE;

static const char* state_to_string(system_state_t s)
{
    switch (s) {
        case SYS_IDLE:     return "IDLE";
        case SYS_ACTIVE:   return "ACTIVE";
        case SYS_DOWNLINK: return "DOWNLINK";
        default:           return "UNKNOWN";
    }
}

static void send_tc(uint8_t service, uint8_t subtype)
{
    pus_packet_t pkt;
    pkt.header.version = 1;
    pkt.header.type    = 1; // TC
    pkt.header.service = service;
    pkt.header.subtype = subtype;
    pkt.header.length  = 0;
    memset(pkt.data, 0, sizeof(pkt.data));
    // Send TC packet to OBC over the established connection
    if (link_send_pkt(obc_fd, &pkt) < 0)
        fprintf(stderr, "Failed to send TC[%d,%d]\n", service, subtype);
}

static void* tm_receiver(void* arg)
{
    int fd = (int)(intptr_t)arg;
    pus_packet_t pkt;
    // BLOCKING: runs in a separate thread, continuously receives TM packets from the satellite
    while (link_recv_pkt(fd, &pkt) == 0) {
        print_tm_output(pkt.header.service, pkt.header.subtype,
                        pkt.data, pkt.header.length);
    }
    return NULL;
}

static void show_menu(void)
{
    printf("\n");
    printf("==================== ERACLEA-1 GROUND CONTROL ====================\n");
    printf("  Satellite state: %-20s\n", state_to_string(ground_state));
    printf("\n");
    printf("1) ACTIVATE         - Activate satellite (IDLE -> ACTIVE)\n");
    printf("2) START DOWNLINK   - Begin data transmission (ACTIVE -> DOWNLINK)\n");
    printf("3) STOP DOWNLINK    - Stop data transmission (DOWNLINK -> ACTIVE)\n");
    printf("4) DEACTIVATE       - Put satellite in standby (ACTIVE -> IDLE)\n");
    printf("0) EXIT             - Close ground station\n");
    printf("\n");
    printf("Choose an option (0-4): ");
}

static int read_choice(void)
{
    int choice;
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return -1;
    }
    while (getchar() != '\n');
    return choice;
}

int main(void)
{
    printf("ERACLEA-1 Ground Control starting...\n");

    // BLOCKING: attempts TCP connection to the OBC server on OBC_HOST:OBC_PORT
    obc_fd = link_ground_connect();
    if (obc_fd < 0) {
        fprintf(stderr, "Cannot connect to OBC on %s:%d. Is it running?\n",
                OBC_HOST, OBC_PORT);
        return 1;
    }
    printf("Connected to OBC.\n");

    // Start receiving telemetry in a background thread while main thread handles user input
    pthread_t t_tm;
    pthread_create(&t_tm, NULL, tm_receiver, (void*)(intptr_t)obc_fd);

    while (1) {
        show_menu();
        int choice = read_choice();

        switch (choice) {
            case 0:
                if (ground_state == SYS_IDLE) {
                    // Gracefully close the connection to OBC and wait for TM receiver thread to exit
                    link_close(obc_fd);
                    pthread_join(t_tm, NULL);
                    return 0;
                } else if (ground_state == SYS_DOWNLINK) {
                    printf("Cannot exit: satellite is in DOWNLINK. Stop downlink first (3), then deactivate (4).\n");
                } else {
                    printf("Cannot exit: satellite is ACTIVE. Deactivate it first (4).\n");
                }
                break;

            case 1:
                if (ground_state == SYS_IDLE) {
                    send_tc(PUS_SVC_MODE_CTRL, PUS_TC_ACTIVATE);
                    ground_state = SYS_ACTIVE;
                    platform_delay_ms(500);
                } else {
                    printf("Cannot activate: state is %s\n", state_to_string(ground_state));
                }
                break;

            case 2:
                if (ground_state == SYS_ACTIVE) {
                    send_tc(PUS_SVC_MODE_CTRL, PUS_TC_START_DOWNLINK);
                    ground_state = SYS_DOWNLINK;
                    platform_delay_ms(500);
                } else {
                    printf("Cannot start downlink: state is %s\n", state_to_string(ground_state));
                }
                break;

            case 3:
                if (ground_state == SYS_DOWNLINK) {
                    send_tc(PUS_SVC_MODE_CTRL, PUS_TC_STOP_DOWNLINK);
                    ground_state = SYS_ACTIVE;
                    platform_delay_ms(500);
                } else {
                    printf("Cannot stop downlink: state is %s\n", state_to_string(ground_state));
                }
                break;

            case 4:
                if (ground_state == SYS_ACTIVE) {
                    send_tc(PUS_SVC_MODE_CTRL, PUS_TC_DEACTIVATE);
                    ground_state = SYS_IDLE;
                    platform_delay_ms(500);
                } else {
                    printf("Cannot deactivate: state is %s\n", state_to_string(ground_state));
                }
                break;

            default:
                printf("  Invalid option. Choose 0-4.\n");
                break;
        }
    }

    return 0;
}
