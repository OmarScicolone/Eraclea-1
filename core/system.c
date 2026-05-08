#include <pthread.h>
#include "system.h"
#include "sensor.h"
#include "buffer.h"
#include "platform.h"
#include "tm_manager.h"
#include "ground_sim.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declaration
void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len);

static system_state_t current_state;
static int error_flag = 0;

static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;

void system_init(void)
{
    sensor_init();
    buffer_init();
    tm_buffer_init();
    current_state = SYS_POWER_OFF;
    printf("[SYSTEM] Initialized - Power OFF\n");
}

void system_power_on(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_POWER_OFF || current_state == SYS_SHUTDOWN) {
        current_state = SYS_POWER_ON;
        error_flag = 0;
        printf("[SYSTEM] Power ON - Ready for data acquisition\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_power_off(void)
{
    pthread_mutex_lock(&state_mutex);
    current_state = SYS_POWER_OFF;
    error_flag = 0;
    pthread_mutex_unlock(&state_mutex);
}

void system_enable_data(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_POWER_ON) {
        current_state = SYS_DATA_ENABLED;
        printf("[SYSTEM] Data acquisition ENABLED\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_disable_data(void)
{
    pthread_mutex_lock(&state_mutex);
    if (current_state == SYS_DATA_ENABLED) {
        current_state = SYS_POWER_ON;
        printf("[SYSTEM] Data acquisition DISABLED\n");
    }
    pthread_mutex_unlock(&state_mutex);
}

void system_shutdown(void)
{
    pthread_mutex_lock(&state_mutex);
    current_state = SYS_SHUTDOWN;
    printf("[SYSTEM] Shutdown initiated\n");
    pthread_mutex_unlock(&state_mutex);
}

system_state_t system_get_state(void)
{
    pthread_mutex_lock(&state_mutex);
    system_state_t state = current_state;
    pthread_mutex_unlock(&state_mutex);
    return state;
}

const char* state_to_string(system_state_t state)
{
    switch (state) {
        case SYS_POWER_OFF: return "SYS_POWER_OFF";
        case SYS_POWER_ON: return "SYS_POWER_ON";
        case SYS_DATA_ENABLED: return "SYS_DATA_ENABLED";
        case SYS_ERROR: return "SYS_ERROR";
        case SYS_SHUTDOWN: return "SYS_SHUTDOWN";
        default: return "UNKNOWN";
    }
}

void* sensor_task(void* arg)
{
    (void)arg;
    int data;
    system_state_t last_state = SYS_POWER_OFF;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        if (state != last_state) {
            printf("[SYSTEM] State changed to %s\n", state_to_string(state));
            last_state = state;
        }

        switch (state) {
            case SYS_POWER_OFF:
                break;

            case SYS_POWER_ON:
                break;

            case SYS_DATA_ENABLED:
                if (sensor_read(&data) == SENSOR_OK) {
                    buffer_push(data);
                }
                break;

            case SYS_ERROR:
                break;

            case SYS_SHUTDOWN:
                return NULL;
        }

        platform_delay_ms(1000);
    }

    return NULL;
}

void* processing_task(void* arg)
{
    (void)arg;
    int data;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        if (state == SYS_DATA_ENABLED) {
            if (buffer_pop(&data) == 0) {
                printf("[PROCESS] data: %d\n", data);
                uint8_t tm_data[2];
                tm_data[0] = (uint8_t)(data & 0xFF);
                tm_data[1] = (uint8_t)((data >> 8) & 0xFF);
                send_tm(130, 1, tm_data, 2);
            }
        } else if (state == SYS_SHUTDOWN) {
            return NULL;
        }

        platform_delay_ms(1500);
    }

    return NULL;
}

void* health_task(void* arg)
{
    (void)arg;
    int tc_counter = 0;
    system_state_t last_state = SYS_POWER_OFF;
    int last_error = 0;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        int err = error_flag;
        pthread_mutex_unlock(&state_mutex);

        if (state != last_state || err != last_error) {
            printf("[HEALTH] State: %s | Error: %d\n", state_to_string(state), err);
            last_state = state;
            last_error = err;
        }

        if (state == SYS_DATA_ENABLED && tc_counter++ % 10 == 0) {
            printf("[HEALTH] Simulating ground command...\n");
            simulate_ground_tc();
        }

        if (state == SYS_SHUTDOWN) {
            return NULL;
        }

        platform_delay_ms(3000);
    }

    return NULL;
}

void* tm_sender_task(void* arg)
{
    (void)arg;
    uint8_t service;
    uint8_t subtype;
    uint8_t data[32];
    uint16_t len;

    while (1) {
        pthread_mutex_lock(&state_mutex);
        system_state_t state = current_state;
        pthread_mutex_unlock(&state_mutex);

        if (state == SYS_SHUTDOWN) {
            return NULL;
        }

        if (tm_buffer_pop(&service, &subtype, data, &len) == 0) {
            print_tm_output(service, subtype, data, len);
        }

        platform_delay_ms(2000);
    }

    return NULL;
}

void show_satellite_menu(void)
{
    printf("\n");
    printf("===================== ERACLEA-1 SATELLITE SIMULATOR =====================\n");
    printf("\n");
    printf("1) SWITCH ON      - Accendi il satellite\n");
    printf("2) ENABLE DATA    - Abilita acquisizione e processamento dati\n");
    printf("3) DISABLE DATA   - Disabilita acquisizione dati (rimane acceso)\n");
    printf("4) SHUTDOWN       - Spegni il satellite\n");
    printf("0) EXIT           - Esci dal simulatore\n");
    printf("\n");
    printf("Scegli un'opzione (0-4): \n");
}

int get_user_command(void)
{
    int choice;
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');
        return COMMAND_INVALID;
    }
    while (getchar() != '\n');
    return choice;
}
