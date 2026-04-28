#include <pthread.h>
#include "system.h"
#include "platform.h"
#include <stdlib.h>
#include <time.h>

int main(void)
{
    printf("MAIN START\n");

    srand(time(NULL));

    system_init();

    pthread_t t_sensor, t_processing, t_health;

    pthread_create(&t_sensor, NULL, sensor_task, NULL);
    pthread_create(&t_processing, NULL, processing_task, NULL);
    pthread_create(&t_health, NULL, health_task, NULL);

    pthread_join(t_sensor, NULL);
    pthread_join(t_processing, NULL);
    pthread_join(t_health, NULL);

    return 0;
}
