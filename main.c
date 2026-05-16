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
    printf("ERACLEA-1 Satellite Control Interface starting... 🛰️\n");

    srand(time(NULL));
    system_init();

    pthread_t t_sensor, t_processing, t_health, t_tm_sender;
    int threads_created = 0;

    while (1) {
        show_satellite_menu();

        int choice = get_user_command();

        switch (choice) {
            case 0: // Exit
                printf("Exiting ERACLEA-1 Control Interface. Goodbye!\n");
                if (threads_created) {
                    printf("Shutting down satellite systems...\n");
                    send_tc(1, 3); // Service 1, Sub 3: Shutdown
                    pthread_join(t_sensor, NULL);
                    pthread_join(t_processing, NULL);
                    pthread_join(t_health, NULL);
                    pthread_join(t_tm_sender, NULL);
                }
                return 0;

            case 1: // Activate
                if (system_get_state() == SYS_IDLE) {
                    printf("Sending ACTIVATE command...\n");
                    if (!threads_created) {
                        pthread_create(&t_sensor,     NULL, sensor_task,     NULL);
                        pthread_create(&t_processing, NULL, processing_task, NULL);
                        pthread_create(&t_health,     NULL, health_task,     NULL);
                        pthread_create(&t_tm_sender,  NULL, tm_sender_task,  NULL);
                        threads_created = 1;
                    }
                    send_tc(1, 1); // Service 1, Sub 1: Switch ON
                } else {
                    printf("⚠️  Cannot activate: satellite not in IDLE state!\n");
                }
                break;

            case 2: // Enable Data
                {
                    system_state_t st = system_get_state();
                    if (st == SYS_ACTIVE) {
                        printf("Sending ENABLE DATA command...\n");
                        send_tc(1, 2); // Service 1, Sub 2: Enable Data
                    } else if (st == SYS_DATA_ENABLED) {
                        printf("⚠️  Data acquisition already enabled!\n");
                    } else {
                        printf("⚠️  Cannot enable data: satellite must be ACTIVE first!\n");
                    }
                }
                break;

            case 3: // Disable Data
                {
                    system_state_t st = system_get_state();
                    if (st == SYS_DATA_ENABLED) {
                        printf("Sending DISABLE DATA command...\n");
                        send_tc(1, 4); // Service 1, Sub 4: Disable Data
                    } else if (st == SYS_ACTIVE) {
                        printf("⚠️  Data acquisition is not enabled!\n");
                    } else {
                        printf("⚠️  Cannot disable data: satellite is not ACTIVE!\n");
                    }
                }
                break;

            case 4: // Enter Idle
                if (!threads_created) {
                    printf("⚠️  Satellite not running!\n");
                    break;
                }
                {
                    system_state_t st = system_get_state();
                    if (st == SYS_DATA_ENABLED) {
                        printf("⚠️  Disable data acquisition first!\n");
                    } else if (st == SYS_ACTIVE) {
                        printf("Sending ENTER IDLE command...\n");
                        send_tc(1, 5); // Service 1, Sub 5: Enter Idle
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
