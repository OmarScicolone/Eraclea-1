#include "buffer.h"
#include <pthread.h>
#include <stdio.h>

static int buffer[BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static int count = 0;
static int overflow_count = 0;

static pthread_mutex_t lock;

void buffer_init(void)
{
    head = tail = count = 0;
    pthread_mutex_init(&lock, NULL);
}

int buffer_push(int data)
{
    pthread_mutex_lock(&lock);

    buffer[head] = data;
    head = (head + 1) % BUFFER_SIZE;

    if (count == BUFFER_SIZE) {
        tail = (tail + 1) % BUFFER_SIZE;
        overflow_count++;
    } else {
        count++;
    }

    pthread_mutex_unlock(&lock);
    return BUFFER_OK;
}

int buffer_pop(int *data)
{
    pthread_mutex_lock(&lock);

    if (count == 0 || !data)
    {
        pthread_mutex_unlock(&lock);
        return BUFFER_EMPTY;
    }

    *data = buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE;
    count--;

    pthread_mutex_unlock(&lock);
    return BUFFER_OK;
}

int buffer_get_count(void)
{
    pthread_mutex_lock(&lock);
    int c = count;
    pthread_mutex_unlock(&lock);
    return c;
}

int buffer_get_overflow_count(void)
{
    pthread_mutex_lock(&lock);
    int ovf = overflow_count;
    pthread_mutex_unlock(&lock);
    return ovf;
}

void buffer_reset_overflow(void)
{
    pthread_mutex_lock(&lock);
    overflow_count = 0;
    pthread_mutex_unlock(&lock);
}
