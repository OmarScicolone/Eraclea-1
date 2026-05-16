#ifndef SYSTEM_H
#define SYSTEM_H

typedef enum {
    SYS_IDLE,       // Satellite in standby, no mission activity
    SYS_ACTIVE,     // Acquiring sensor data onboard
    SYS_DOWNLINK,   // Transmitting data to ground station
    SYS_ERROR,      // Error state (not yet implemented)
    SYS_SHUTDOWN    // Internal: simulation cleanup
} system_state_t;

void system_init(void);
void system_activate(void);        // IDLE -> ACTIVE
void system_deactivate(void);      // ACTIVE -> IDLE
void system_start_downlink(void);  // ACTIVE -> DOWNLINK
void system_stop_downlink(void);   // DOWNLINK -> ACTIVE
void system_shutdown(void);

system_state_t system_get_state(void);

// OBC tasks (RTOS-like threads)
void* sensor_task(void* arg);
void* processing_task(void* arg);
void* health_task(void* arg);
void* tm_sender_task(void* arg);   // arg = int ground_fd cast to void*

#endif
