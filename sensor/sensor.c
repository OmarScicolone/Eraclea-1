#include "sensor.h"
#include <stdlib.h>

static int counter = 0;
static int ok_count = 0;
static int error_count = 0;

void sensor_init(void)
{
    counter = 0;
    ok_count = 0;
    error_count = 0;
}

int sensor_read(int *data)
{
    if (!data) {
        error_count++;
        return SENSOR_ERROR;
    }

    if (counter++ % 10 == 0) {
        error_count++;
        return SENSOR_ERROR;
    }

    *data = rand() % 100;
    ok_count++;
    return SENSOR_OK;
}

int sensor_get_ok_count(void)    { return ok_count; }
int sensor_get_error_count(void) { return error_count; }
