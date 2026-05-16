#include "tm_manager.h"
#include <stdatomic.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include "link.h"
#include "platform.h"

#define TM_BUFFER_SIZE 32

typedef struct {
    uint8_t  service;
    uint8_t  subtype;
    uint8_t  data[32];
    uint16_t len;
} tm_packet_t;

static tm_packet_t      tm_buffer[TM_BUFFER_SIZE];
static int              head       = 0;
static int              tail       = 0;
static int              count      = 0;
static int              sent_count = 0;
static pthread_mutex_t  lock       = PTHREAD_MUTEX_INITIALIZER;

void tm_buffer_init(void)
{
    pthread_mutex_lock(&lock);
    head = tail = count = sent_count = 0;
    memset(tm_buffer, 0, sizeof(tm_buffer));
    pthread_mutex_unlock(&lock);
}

static int tm_buffer_push(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len)
{
    pthread_mutex_lock(&lock);

    if (count >= TM_BUFFER_SIZE) {
        pthread_mutex_unlock(&lock);
        printf("[TM] Buffer full — packet dropped (srv=%d sub=%d)\n", service, subtype);
        return TM_FULL;
    }

    if (len > 32) len = 32;

    tm_buffer[head].service = service;
    tm_buffer[head].subtype = subtype;
    tm_buffer[head].len     = len;
    if (data && len > 0)
        memcpy(tm_buffer[head].data, data, len);

    head = (head + 1) % TM_BUFFER_SIZE;
    count++;

    pthread_mutex_unlock(&lock);
    return TM_OK;
}

static int tm_buffer_pop(uint8_t* service, uint8_t* subtype, uint8_t* data, uint16_t* len)
{
    pthread_mutex_lock(&lock);

    if (count == 0 || !service || !subtype || !len) {
        pthread_mutex_unlock(&lock);
        return TM_EMPTY;
    }

    *service = tm_buffer[tail].service;
    *subtype = tm_buffer[tail].subtype;
    *len     = tm_buffer[tail].len;
    if (data && *len > 0)
        memcpy(data, tm_buffer[tail].data, *len);

    tail = (tail + 1) % TM_BUFFER_SIZE;
    count--;
    sent_count++;

    pthread_mutex_unlock(&lock);
    return TM_OK;
}

int tm_get_sent_count(void)
{
    pthread_mutex_lock(&lock);
    int s = sent_count;
    pthread_mutex_unlock(&lock);
    return s;
}

void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len)
{
    tm_buffer_push(service, subtype, data, len);
}

// ── TM sender task ────────────────────────────────────────────────────────────

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
