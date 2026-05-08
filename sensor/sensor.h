#ifndef SENSOR_H
#define SENSOR_H

#define SENSOR_OK 0
#define SENSOR_ERROR (-1)

void sensor_init(void);
int sensor_read(int *data);

#endif
