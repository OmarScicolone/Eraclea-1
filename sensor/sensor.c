#include "sensor.h"
#include <stdlib.h>

static int counter = 0;

void sensor_init(void)
{
    counter = 0;
}

int sensor_read(int *data)
{
    if (!data)
        return SENSOR_ERROR;

    // errore simulato
    if (counter++ % 10 == 0)
        return SENSOR_ERROR;

    *data = rand() % 100;
    return SENSOR_OK;
}
