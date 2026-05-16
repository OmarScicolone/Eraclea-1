#include "tm_manager.h"
#include "pus.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#define TM_BUFFER_SIZE 32

typedef struct {
    uint8_t service;
    uint8_t subtype;
    uint8_t data[32];
    uint16_t len;
} tm_packet_t;

static tm_packet_t tm_buffer[TM_BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static int count = 0;
static int sent_count = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void tm_buffer_init(void)
{
    pthread_mutex_lock(&lock);
    head = tail = count = sent_count = 0;
    memset(tm_buffer, 0, sizeof(tm_buffer));
    pthread_mutex_unlock(&lock);
}

int tm_buffer_push(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len)
{
    pthread_mutex_lock(&lock);

    if (count >= TM_BUFFER_SIZE) {
        pthread_mutex_unlock(&lock);
        printf("[TM_MGR] Buffer full!\n");
        return TM_FULL;
    }

    if (len > 32) len = 32;

    tm_buffer[head].service = service;
    tm_buffer[head].subtype = subtype;
    tm_buffer[head].len = len;
    
    if (data && len > 0)
        memcpy(tm_buffer[head].data, data, len);

    head = (head + 1) % TM_BUFFER_SIZE;
    count++;

    printf("[TM_MGR] Pushed TM (srv=%d, sub=%d, len=%d)\n", service, subtype, len);

    pthread_mutex_unlock(&lock);
    return TM_OK;
}

int tm_buffer_pop(uint8_t* service, uint8_t* subtype, uint8_t* data, uint16_t* len)
{
    pthread_mutex_lock(&lock);

    if (count == 0 || !service || !subtype || !len) {
        pthread_mutex_unlock(&lock);
        return TM_EMPTY;
    }

    *service = tm_buffer[tail].service;
    *subtype = tm_buffer[tail].subtype;
    *len = tm_buffer[tail].len;

    if (data && *len > 0)
        memcpy(data, tm_buffer[tail].data, *len);

    tail = (tail + 1) % TM_BUFFER_SIZE;
    count--;
    sent_count++;

    printf("[TM_MGR] Popped TM (srv=%d, sub=%d, len=%d)\n", *service, *subtype, *len);

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