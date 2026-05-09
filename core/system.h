#ifndef SYSTEM_H
#define SYSTEM_H

#include <pthread.h>

#define COMMAND_INVALID (-1)

typedef enum {
    SYS_IDLE,         // Sistema in standby
    SYS_OPERATIVE,    // Acceso ma senza acquisizione dati
    SYS_DATA_ENABLED, // Acquisizione e processamento attivi
    SYS_ERROR,        // Stato di errore
    SYS_SHUTDOWN      // Spegnimento in corso
} system_state_t;

void system_init(void);
void system_power_on(void);
void system_power_off(void);
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