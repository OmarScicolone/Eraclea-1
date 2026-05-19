#include "buffer.h"
#include <pthread.h>

// Ring buffer FIFO (circular queue): data pushed at head, popped from tail.
// When full, oldest data (at tail) is overwritten, not discarded.
// Protected by mutex for thread-safe concurrent access.
static int buffer[BUFFER_SIZE];
static int head           = 0;
static int tail           = 0;
static int count          = 0;
static int overflow_count = 0;

static pthread_mutex_t buf_lock = PTHREAD_MUTEX_INITIALIZER;

void buffer_init(void)
{
    pthread_mutex_lock(&buf_lock);
    head = tail = count = overflow_count = 0;
    pthread_mutex_unlock(&buf_lock);
}

int buffer_push(int data)
{
    pthread_mutex_lock(&buf_lock);

    buffer[head] = data;
    head = (head + 1) % BUFFER_SIZE;

    int ret;
    if (count == BUFFER_SIZE) {
        // Buffer full: overwrite oldest sample, advance tail
        tail = (tail + 1) % BUFFER_SIZE;
        overflow_count++;
        ret = BUFFER_OVERFLOW;
    } else {
        count++;
        ret = BUFFER_OK;
    }

    pthread_mutex_unlock(&buf_lock);
    return ret;
}

int buffer_pop(int *data)
{
    pthread_mutex_lock(&buf_lock);

    if (count == 0 || !data) {
        pthread_mutex_unlock(&buf_lock);
        return BUFFER_EMPTY;
    }

    *data = buffer[tail];
    tail  = (tail + 1) % BUFFER_SIZE;
    count--;

    pthread_mutex_unlock(&buf_lock);
    return BUFFER_OK;
}

int buffer_get_count(void)
{
    pthread_mutex_lock(&buf_lock);
    int c = count;
    pthread_mutex_unlock(&buf_lock);
    return c;
}

int buffer_get_overflow_count(void)
{
    pthread_mutex_lock(&buf_lock);
    int ovf = overflow_count;
    pthread_mutex_unlock(&buf_lock);
    return ovf;
}
