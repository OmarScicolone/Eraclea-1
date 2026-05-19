#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "system.h"
#include "tc_handler.h"
#include "tm_manager.h"
#include "link.h"

// Shared with tm_sender_task: current ground socket fd, -1 if no ground connected.
// _Atomic ensures reads/writes from different threads are indivisible (no data race).
static _Atomic int ground_fd = ATOMIC_VAR_INIT(-1);

int main(void)
{
    printf("ERACLEA-1 OBC starting...\n");
    // Seed RNG with current time for sensor randomness
    srand((unsigned int)time(NULL));

    // Initialize OBC subsystems: sensor, data buffer, telemetry queue
    system_init();

    // Start OBC tasks
    pthread_t t_sensor, t_processing, t_health, t_tm_sender;
    pthread_create(&t_sensor,     NULL, sensor_task,     NULL);
    pthread_create(&t_processing, NULL, processing_task, NULL);
    pthread_create(&t_health,     NULL, health_task,     NULL);
    pthread_create(&t_tm_sender,  NULL, tm_sender_task,  (void*)&ground_fd);

    // Open TCP server socket on OBC_PORT and start listening
    int server_fd = link_obc_init();
    if (server_fd < 0)
        return 1;

    // Accept ground connections indefinitely — satellite keeps running between sessions
    while (1) {
        printf("\nListening for ground station on port %d...\n", OBC_PORT);
        // BLOCKING: waits until a ground station establishes a TCP connection
        int fd = link_obc_accept(server_fd);
        if (fd < 0) {
            fprintf(stderr, "Accept failed, retrying...\n");
            continue;
        }

        atomic_store(&ground_fd, fd);   // notify tm_sender_task: a ground station is now connected
        printf("Ground station connected.\n");

        // BLOCKING: receives TC packets one by one until the ground station disconnects
        pus_packet_t pkt;
        while (link_recv_pkt(fd, &pkt) == 0) {
            handle_tc(&pkt);
        }

        atomic_store(&ground_fd, -1);   // notify tm_sender_task: no ground connected, stop sending TM
        // Gracefully close the connection with this ground station session
        link_close(fd);
        printf("Ground station disconnected.\n");
    }

    return 0;
}
