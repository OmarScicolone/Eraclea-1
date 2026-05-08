#include <pthread.h>
#include "system.h"
#include "platform.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(void)
{
    printf("🚀 ERACLEA-1 Satellite Simulator Starting...\n");

    srand(time(NULL));
    system_init();

    // Thread handles
    pthread_t t_sensor, t_processing, t_health, t_tm_sender;
    int threads_created = 0;

    while (1) {
        show_satellite_menu();

        int choice = get_user_command();

        switch (choice) {
            case 0: // Exit
                printf("👋 Exiting ERACLEA-1 Simulator. Goodbye!\n");
                if (threads_created) {
                    system_shutdown();
                    sleep(2);
                }
                return 0;

            case 1: // Switch ON
                if (!threads_created) {
                    printf("🚀 Starting satellite systems...\n");
                    system_power_on();
                    pthread_create(&t_sensor, NULL, sensor_task, NULL);
                    pthread_create(&t_processing, NULL, processing_task, NULL);
                    pthread_create(&t_health, NULL, health_task, NULL);
                    pthread_create(&t_tm_sender, NULL, tm_sender_task, NULL);
                    threads_created = 1;
                    printf("✅ Satellite systems online!\n");
                } else {
                    printf("⚠️  Satellite already running!\n");
                }
                break;

            case 2: // Enable Data
                if (threads_created) {
                    printf("📊 Enabling data acquisition...\n");
                    system_enable_data();
                } else {
                    printf("⚠️ Satellite not switched on.\n");
                }
                break;

            case 3: // Disable Data
                if (threads_created) {
                    printf("📊 Disabling data acquisition...\n");
                    system_disable_data();
                } else {
                    printf("⚠️ Satellite not switched on.\n");
                }
                break;

            case 4: // Shutdown
                if (threads_created) {
                    printf("🛑 Shutting down satellite...\n");
                    system_shutdown();
                    pthread_join(t_sensor, NULL);
                    pthread_join(t_processing, NULL);
                    pthread_join(t_health, NULL);
                    pthread_join(t_tm_sender, NULL);
                    system_power_off();
                    threads_created = 0;
                    printf("✅ Satellite shutdown complete!\n");
                } else {
                    printf("⚠️  Satellite not running!\n");
                }
                break;

            default:
                printf("❌ Invalid option! Choose 0-4.\n");
                break;
        }

        if (choice != 0) {
            printf("\nPress Enter to return to mission control...\n");
            getchar();
        }
    }

    return 0;
}