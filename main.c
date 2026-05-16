#include <pthread.h>
#include "platform.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "system.h"
#include "tc_handler.h"
#include "pus.h"

static void send_tc(uint8_t service, uint8_t subtype)
{
    pus_packet_t pkt;
    pkt.header.version = 1;
    pkt.header.type    = 1; // TC
    pkt.header.service = service;
    pkt.header.subtype = subtype;
    pkt.header.length  = 0;
    handle_tc(&pkt);
}

int main(void)
{
    printf("ERACLEA-1 Ground Control starting... 🛰️\n");

    srand(time(NULL));
    system_init();

    pthread_t t_sensor, t_processing, t_health, t_tm_sender;
    int threads_created = 0;

    while (1) {
        show_satellite_menu();

        int choice = get_user_command();

        switch (choice) {
            case 0: // Exit — close ground station, satellite stays in current state
                printf("Closing ground station...\n");
                if (threads_created) {
                    system_shutdown(); // internal cleanup only, not a satellite TC
                    pthread_join(t_sensor, NULL);
                    pthread_join(t_processing, NULL);
                    pthread_join(t_health, NULL);
                    pthread_join(t_tm_sender, NULL);
                }
                printf("Ground station offline. Satellite remains in orbit.\n");
                return 0;

            case 1: // Activate (IDLE → ACTIVE)
                if (system_get_state() == SYS_IDLE) {
                    if (!threads_created) {
                        pthread_create(&t_sensor,     NULL, sensor_task,     NULL);
                        pthread_create(&t_processing, NULL, processing_task, NULL);
                        pthread_create(&t_health,     NULL, health_task,     NULL);
                        pthread_create(&t_tm_sender,  NULL, tm_sender_task,  NULL);
                        threads_created = 1;
                    }
                    send_tc(1, 1); // Service 1, Sub 1: Activate
                } else {
                    printf("⚠️  Cannot activate: satellite not in IDLE state!\n");
                }
                break;

            case 2: // Start Downlink (ACTIVE → DOWNLINK)
                {
                    system_state_t st = system_get_state();
                    if (st == SYS_ACTIVE) {
                        send_tc(1, 2); // Service 1, Sub 2: Start Downlink
                    } else if (st == SYS_DOWNLINK) {
                        printf("⚠️  Downlink already active!\n");
                    } else {
                        printf("⚠️  Cannot start downlink: satellite must be ACTIVE first!\n");
                    }
                }
                break;

            case 3: // Stop Downlink (DOWNLINK → ACTIVE)
                {
                    system_state_t st = system_get_state();
                    if (st == SYS_DOWNLINK) {
                        send_tc(1, 4); // Service 1, Sub 4: Stop Downlink
                    } else if (st == SYS_ACTIVE) {
                        printf("⚠️  Downlink is not active!\n");
                    } else {
                        printf("⚠️  Cannot stop downlink: satellite is not in DOWNLINK state!\n");
                    }
                }
                break;

            case 4: // Deactivate (ACTIVE → IDLE)
                if (!threads_created) {
                    printf("⚠️  Satellite not running!\n");
                    break;
                }
                {
                    system_state_t st = system_get_state();
                    if (st == SYS_DOWNLINK) {
                        printf("⚠️  Stop downlink first!\n");
                    } else if (st == SYS_ACTIVE) {
                        send_tc(1, 5); // Service 1, Sub 5: Deactivate
                    } else {
                        printf("⚠️  Already in idle or unavailable state.\n");
                    }
                }
                break;

            default:
                printf("Invalid option! Choose 0-4.\n");
                break;
        }
    }

    return 0;
}
