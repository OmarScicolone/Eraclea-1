#include "buffer.h"
#include <pthread.h>

static int buffer[BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static int count = 0;

static pthread_mutex_t lock;

void buffer_init(void)
{
    head = tail = count = 0;
    pthread_mutex_init(&lock, NULL);
}

int buffer_push(int data)
{
    pthread_mutex_lock(&lock);
    printf("[BUFFER] lock push\n");

    if (count == BUFFER_SIZE)
    {
        pthread_mutex_unlock(&lock);
        return -1;
    }

    buffer[head] = data;
    head = (head + 1) % BUFFER_SIZE;
    count++;

    pthread_mutex_unlock(&lock);
    printf("[BUFFER] unlock push\n");
    return 0;
}

int buffer_pop(int *data)
{
    printf("[BUFFER] try pop\n");

    pthread_mutex_lock(&lock);
    printf("[BUFFER] lock pop\n");

    if (count == 0 || !data)
    {
    	printf("[BUFFER] unlock\n");
        pthread_mutex_unlock(&lock);
        return -1;
    }

    *data = buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE;
    count--;

    pthread_mutex_unlock(&lock);
    printf("[BUFFER] unlock pop\n");
    return 0;
}
