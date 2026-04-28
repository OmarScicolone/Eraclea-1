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
        return -1;

    // errore simulato
    if (counter++ % 10 == 0)
        return -1;

    *data = rand() % 100;
    return 0;
}
