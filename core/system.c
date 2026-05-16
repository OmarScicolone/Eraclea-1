#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "system.h"
#include "sensor.h"
#include "buffer.h"
#include "platform.h"
#include "tm_manager.h"
#include "link.h"

static system_state_t  current_state;
static time_t          activate_time = 0;
static pthread_mutex_t state_mutex   = PTHREAD_MUTEX_INITIALIZER;

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void system_init(void)
{
    sensor_init();
    buffer_init();
    tm_buffer_init();
    current_state = SYS_IDLE;
    printf("System initialized — state: IDLE\n");
}

// ── State transitions ─────────────────────────────────────────────────────────

void system_activate(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_IDLE) {
        current_state = SYS_ACTIVE;
        activate_time = time(NULL);
        printf("State -> ACTIVE\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_deactivate(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_ACTIVE) {
        current_state = SYS_IDLE;
        activate_time = 0;
        printf("State -> IDLE\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_start_downlink(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_ACTIVE) {
        current_state = SYS_DOWNLINK;
        printf("State -> DOWNLINK\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_stop_downlink(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_DOWNLINK) {
        current_state = SYS_ACTIVE;
        printf("State -> ACTIVE\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

system_state_t system_get_state(void)
{
    pthread_mutex_lock(&state_mutex);
    system_state_t s = current_state;
    pthread_mutex_unlock(&state_mutex);
    return s;
}

uint32_t system_get_uptime(void)
{
    pthread_mutex_lock(&state_mutex);
    uint32_t uptime = (activate_time > 0)
                      ? (uint32_t)difftime(time(NULL), activate_time)
                      : 0;
    pthread_mutex_unlock(&state_mutex);
    return uptime;
}

// ── HK report builder (shared by health_task and TC[3,27] handler) ────────────

uint8_t system_build_hk_report(uint8_t* buf)
{
    uint32_t uptime     = system_get_uptime();
    uint16_t sensor_ok  = (uint16_t)sensor_get_ok_count();
    uint16_t sensor_err = (uint16_t)sensor_get_error_count();
    uint8_t  buf_ovf    = (uint8_t)buffer_get_overflow_count();
    uint8_t  buf_occ    = (uint8_t)buffer_get_count();
    uint16_t tm_sent    = (uint16_t)tm_get_sent_count();

    buf[0]  = (uint8_t)(uptime >> 24);
    buf[1]  = (uint8_t)(uptime >> 16);
    buf[2]  = (uint8_t)(uptime >> 8);
    buf[3]  = (uint8_t)(uptime);
    buf[4]  = (uint8_t)(sensor_ok >> 8);
    buf[5]  = (uint8_t)(sensor_ok);
    buf[6]  = (uint8_t)(sensor_err >> 8);
    buf[7]  = (uint8_t)(sensor_err);
    buf[8]  = buf_ovf;
    buf[9]  = buf_occ;
    buf[10] = (uint8_t)(tm_sent >> 8);
    buf[11] = (uint8_t)(tm_sent);
    return 12;
}

// ── OBC Tasks ─────────────────────────────────────────────────────────────────

void* sensor_task(void* arg)
{
    (void)arg;
    int data;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        switch (state) {
            case SYS_ACTIVE:
            case SYS_DOWNLINK:
                if (sensor_read(&data) == SENSOR_OK)
                    buffer_push(data);
                break;

            case SYS_IDLE:
            default:
                break;
        }

        platform_delay_ms(2000);
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

        if (state == SYS_DOWNLINK) {
            if (buffer_pop(&data) == BUFFER_OK) {
                uint8_t payload[2];
                payload[0] = (uint8_t)(data & 0xFF);
                payload[1] = (uint8_t)((data >> 8) & 0xFF);
                send_tm(PUS_SVC_SENSOR_DATA, PUS_TM_SENSOR_READING, payload, 2);
            }
        }

        platform_delay_ms(800);
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

        if (state == SYS_DOWNLINK) {
            uint8_t hk[12];
            uint8_t len = system_build_hk_report(hk);
            send_tm(PUS_SVC_HK, PUS_TM_HK_REPORT, hk, len);
        }

        platform_delay_ms(5000);
    }

    return NULL;
}

static const char* tm_describe(uint8_t svc, uint8_t sub)
{
    if (svc == PUS_SVC_VERIFICATION && sub == PUS_TM_ACCEPT_OK)   return "Verification: TC Accepted";
    if (svc == PUS_SVC_VERIFICATION && sub == PUS_TM_ACCEPT_FAIL) return "Verification: TC Rejected";
    if (svc == PUS_SVC_HK           && sub == PUS_TM_HK_REPORT)   return "Housekeeping Report";
    if (svc == PUS_SVC_TEST         && sub == PUS_TM_I_AM_ALIVE)  return "Connection Test: I-Am-Alive";
    return "Unknown TM";
}

void* tm_sender_task(void* arg)
{
    _Atomic int* fd_ptr = (_Atomic int*)arg;
    uint8_t  service, subtype, data[32];
    uint16_t len;

    while (1) {
        int fd = atomic_load(fd_ptr);

        if (fd >= 0 && tm_buffer_pop(&service, &subtype, data, &len) == TM_OK) {
            pus_packet_t pkt;
            pkt.header.version = 1;
            pkt.header.type    = 0; // TM
            pkt.header.service = service;
            pkt.header.subtype = subtype;
            pkt.header.length  = len;
            memset(pkt.data, 0, sizeof(pkt.data));
            memcpy(pkt.data, data, len);

            if (link_send_pkt(fd, &pkt) < 0) {
                printf("\nTM [%d,%d] send failed\n", service, subtype);
            } else if (service == PUS_SVC_SENSOR_DATA && subtype == PUS_TM_SENSOR_READING && len >= 2) {
                int16_t val = (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
                printf("\nTM [%d,%d] -> Sensor Data (value=%d)\n", service, subtype, val);
            } else {
                printf("\nTM [%d,%d] -> %s\n", service, subtype, tm_describe(service, subtype));
            }
        }

        platform_delay_ms(200);
    }

    return NULL;
}
