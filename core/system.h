#ifndef SYSTEM_H
#define SYSTEM_H

#include <pthread.h>

#define COMMAND_INVALID (-1)

typedef enum {
    SYS_IDLE,         // System in idle
    SYS_ACTIVE,       // Powered and ready, but data acquisition disabled
    SYS_DATA_ENABLED, // Data acquisition and processing active
    SYS_ERROR,        // Error state
    SYS_SHUTDOWN      // Shutdown in progress
} system_state_t;

void system_init(void);
void system_power_on(void);
void system_in_idle(void);
void system_enable_data(void);
void system_disable_data(void);
void system_shutdown(void);

// Get current system state
system_state_t system_get_state(void);
const char* state_to_string(system_state_t state);

// Tasks (RTOS-like)
void* sensor_task(void* arg);
void* processing_task(void* arg);
void* health_task(void* arg);
void* tm_sender_task(void* arg);

// UI functions
void show_satellite_menu(void);
int get_user_command(void);

#endif