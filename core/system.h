#ifndef SYSTEM_H
#define SYSTEM_H

#include <pthread.h>

typedef enum {
    SYS_INIT,
    SYS_IDLE,
    SYS_ERROR
} system_state_t;

void system_init(void);

// task (RTOS-like)
void* sensor_task(void* arg);
void* processing_task(void* arg);
void* health_task(void* arg);

#endif