#ifndef SENSOR_H
#define SENSOR_H

#define SENSOR_OK 0
#define SENSOR_ERROR (-1)

void sensor_init(void);
int sensor_read(int *data);
int sensor_get_ok_count(void);
int sensor_get_error_count(void);

#endif
