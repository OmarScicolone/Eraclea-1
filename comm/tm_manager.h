#ifndef TM_MANAGER_H
#define TM_MANAGER_H

#include <stdint.h>

#define TM_OK    0
#define TM_EMPTY (-1)
#define TM_FULL  (-2)

void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len);

void tm_buffer_init(void);
int  tm_get_sent_count(void);

void* tm_sender_task(void* arg);   // arg = _Atomic int* ground_fd

#endif
