#include <pthread.h>
#include "system.h"
#include "sensor.h"
#include "buffer.h"
#include "platform.h"
#include <stdio.h>

static system_state_t current_state;
static int error_flag = 0;

static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

void system_init(void)
{
    sensor_init();
    buffer_init();
    current_state = SYS_INIT;
}

void* sensor_task(void* arg)
{
    int data;

    while (1)
{
    pthread_mutex_lock(&state_mutex);
    system_state_t state = current_state;
    pthread_mutex_unlock(&state_mutex);

    printf("[SENSOR] tick state=%d\n", state);

    switch (state)
    {
        case SYS_INIT:
            pthread_mutex_lock(&state_mutex);
            current_state = SYS_IDLE;
            pthread_mutex_unlock(&state_mutex);
            break;

        case SYS_IDLE:
        {
            int data = rand() % 100;

            //if (sensor_read(&data) == 0)
            //{
            
                printf("[SENSOR] pushing %d\n", data);
		if (buffer_push(data) == 0){
		    printf("[SENSOR] pushed OK\n");
		}
                if (buffer_push(data) != 0)
                {
                    pthread_mutex_lock(&state_mutex);
                    error_flag = 1;
                    current_state = SYS_ERROR;
                    pthread_mutex_unlock(&state_mutex);
                }
            //}
            break;
        }
    }

    printf("[SENSOR] alive\n");
    platform_delay_ms(200);
}

    return NULL;
}

void* processing_task(void* arg)
{
    int data;

    while (1)
    {
    printf("[PROCESS] tick\n");
        if (buffer_pop(&data) == 0)
	{
    		printf("[PROCESS] data: %d\n", data);
	}
		else
	{
    		printf("[PROCESS] empty\n");
	}

        platform_delay_ms(300);
    }

    return NULL;
}

void* health_task(void* arg)
{
    while (1)
    {
    	pthread_mutex_lock(&state_mutex);
    	
    	system_state_t state = current_state;
    	int err = error_flag;
    	
    	pthread_mutex_unlock(&state_mutex);
    	
        printf("[HEALTH] State: %d | Error: %d\n", state, error_flag);
        
        platform_delay_ms(1000);
    }

    return NULL;
}
