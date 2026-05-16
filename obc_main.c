#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "system.h"
#include "tc_handler.h"
#include "link.h"
#include "pus.h"

// Shared with tm_sender_task: current ground socket fd, -1 if no ground connected
static volatile int ground_fd = -1;

int main(void)
{
    printf("[OBC] ERACLEA-1 OBC starting...\n");
    srand((unsigned int)time(NULL));
    system_init();

    // Start OBC tasks once — they run for the lifetime of the satellite
    pthread_t t_sensor, t_processing, t_health, t_tm_sender;
    pthread_create(&t_sensor,     NULL, sensor_task,     NULL);
    pthread_create(&t_processing, NULL, processing_task, NULL);
    pthread_create(&t_health,     NULL, health_task,     NULL);
    pthread_create(&t_tm_sender,  NULL, tm_sender_task,  (void*)&ground_fd);

    // Accept ground connections indefinitely — satellite keeps running between sessions
    while (1) {
        printf("[OBC] Listening for ground station on port %d...\n", OBC_PORT);
        int fd = link_obc_listen();
        if (fd < 0) {
            fprintf(stderr, "[OBC] Accept failed, retrying...\n");
            continue;
        }

        ground_fd = fd;
        printf("[OBC] Ground station connected.\n");

        // Receive TC packets until ground disconnects
        pus_packet_t pkt;
        while (link_recv_pkt(ground_fd, &pkt) == 0) {
            handle_tc(&pkt);
        }

        ground_fd = -1;
        link_close(fd);
        printf("[OBC] Ground station disconnected.\n");
    }

    return 0;
}
