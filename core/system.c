#include <pthread.h>
#include "system.h"
#include "sensor.h"
#include "buffer.h"
#include "platform.h"
#include "tm_manager.h"
#include "link.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Forward declaration (defined in tm_manager.c)
void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len);

static system_state_t current_state;
static time_t activate_time = 0;

static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

void system_init(void)
{
    sensor_init();
    buffer_init();
    tm_buffer_init();
    current_state = SYS_IDLE;
    printf("[SYSTEM] Initialized - Idle\n");
}

void system_activate(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_IDLE) {
        current_state = SYS_ACTIVE;
        activate_time = time(NULL);
        printf("[SYSTEM] ACTIVE - Acquiring sensor data onboard\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_deactivate(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_ACTIVE) {
        current_state = SYS_IDLE;
        printf("[SYSTEM] IDLE - Satellite in standby\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_start_downlink(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_ACTIVE) {
        current_state = SYS_DOWNLINK;
        printf("[SYSTEM] DOWNLINK - Transmitting data to ground station\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_stop_downlink(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_DOWNLINK) {
        current_state = SYS_ACTIVE;
        printf("[SYSTEM] ACTIVE - Downlink stopped, still acquiring\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_shutdown(void)
{
    pthread_mutex_lock(&state_mutex);
    current_state = SYS_SHUTDOWN;
    printf("[SYSTEM] Simulation shutdown\n");
    pthread_mutex_unlock(&state_mutex);
}

system_state_t system_get_state(void)
{
    pthread_mutex_lock(&state_mutex);
    system_state_t state = current_state;
    pthread_mutex_unlock(&state_mutex);
    return state;
}

void* sensor_task(void* arg)
{
    (void)arg;
    int data;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        switch (state) {
            case SYS_IDLE:
                break;

            case SYS_ACTIVE:
            case SYS_DOWNLINK:
                if (sensor_read(&data) == SENSOR_OK) {
                    buffer_push(data);
                }
                break;

            case SYS_ERROR:
                break;

            case SYS_SHUTDOWN:
                return NULL;
        }

        platform_delay_ms(1000);
    }

    return NULL;
}

void* processing_task(void* arg)
{
    (void)arg;
    int data;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        if (state == SYS_SHUTDOWN)
            return NULL;

        if (state == SYS_DOWNLINK) {
            if (buffer_pop(&data) == 0) {
                printf("[PROCESS] data: %d\n", data);
                uint8_t tm_data[2];
                tm_data[0] = (uint8_t)(data & 0xFF);
                tm_data[1] = (uint8_t)((data >> 8) & 0xFF);
                send_tm(130, 1, tm_data, 2);
            }
        }

        platform_delay_ms(1500);
    }

    return NULL;
}

void* health_task(void* arg)
{
    (void)arg;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        if (state == SYS_SHUTDOWN)
            return NULL;

        if (state == SYS_DOWNLINK) {
            uint32_t uptime = (activate_time > 0)
                              ? (uint32_t)difftime(time(NULL), activate_time)
                              : 0;
            uint16_t sensor_ok  = (uint16_t)sensor_get_ok_count();
            uint16_t sensor_err = (uint16_t)sensor_get_error_count();
            uint8_t  buf_ovf    = (uint8_t)buffer_get_overflow_count();
            uint8_t  buf_occ    = (uint8_t)buffer_get_count();
            uint16_t tm_sent    = (uint16_t)tm_get_sent_count();

            uint8_t hk[12];
            hk[0]  = (uint8_t)(uptime >> 24);
            hk[1]  = (uint8_t)(uptime >> 16);
            hk[2]  = (uint8_t)(uptime >> 8);
            hk[3]  = (uint8_t)(uptime);
            hk[4]  = (uint8_t)(sensor_ok >> 8);
            hk[5]  = (uint8_t)(sensor_ok);
            hk[6]  = (uint8_t)(sensor_err >> 8);
            hk[7]  = (uint8_t)(sensor_err);
            hk[8]  = buf_ovf;
            hk[9]  = buf_occ;
            hk[10] = (uint8_t)(tm_sent >> 8);
            hk[11] = (uint8_t)(tm_sent);

            send_tm(3, 25, hk, sizeof(hk));
        }

        platform_delay_ms(5000);
    }

    return NULL;
}

void* tm_sender_task(void* arg)
{
    volatile int* fd_ptr = (volatile int*)arg;
    uint8_t service;
    uint8_t subtype;
    uint8_t data[32];
    uint16_t len;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        if (state == SYS_SHUTDOWN)
            return NULL;

        int fd = *fd_ptr;
        if (fd >= 0 && tm_buffer_pop(&service, &subtype, data, &len) == 0) {
            pus_packet_t pkt;
            pkt.header.version = 1;
            pkt.header.type    = 0; // TM
            pkt.header.service = service;
            pkt.header.subtype = subtype;
            pkt.header.length  = len;
            memset(pkt.data, 0, sizeof(pkt.data));
            memcpy(pkt.data, data, len);
            if (link_send_pkt(fd, &pkt) < 0)
                printf("[TM_SENDER] Failed to send TM to ground\n");
            else
                printf("[TM_SENDER] Sent TM (srv=%d sub=%d len=%d) to ground\n",
                       service, subtype, len);
        }

        platform_delay_ms(2000);
    }

    return NULL;
}
