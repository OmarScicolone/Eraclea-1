#include "sensor.h"
#include <stdlib.h>
#include <pthread.h>

static int counter     = 0;
static int ok_count    = 0;
static int error_count = 0;

static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void sensor_init(void)
{
    pthread_mutex_lock(&counter_lock);
    counter     = 0;
    ok_count    = 0;
    error_count = 0;
    pthread_mutex_unlock(&counter_lock);
}

int sensor_read(int *data)
{
    if (!data) {
        pthread_mutex_lock(&counter_lock);
        error_count++;
        pthread_mutex_unlock(&counter_lock);
        return SENSOR_ERROR;
    }

    // Simulate a read error every 10 acquisitions
    if (counter++ % 10 == 0) {
        pthread_mutex_lock(&counter_lock);
        error_count++;
        pthread_mutex_unlock(&counter_lock);
        return SENSOR_ERROR;
    }

    *data = rand() % 100;

    pthread_mutex_lock(&counter_lock);
    ok_count++;
    pthread_mutex_unlock(&counter_lock);
    return SENSOR_OK;
}

int sensor_get_ok_count(void)
{
    pthread_mutex_lock(&counter_lock);
    int v = ok_count;
    pthread_mutex_unlock(&counter_lock);
    return v;
}

int sensor_get_error_count(void)
{
    pthread_mutex_lock(&counter_lock);
    int v = error_count;
    pthread_mutex_unlock(&counter_lock);
    return v;
}
