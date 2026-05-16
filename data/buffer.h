#ifndef BUFFER_H
#define BUFFER_H

#define BUFFER_SIZE 16

#define BUFFER_OK       0
#define BUFFER_EMPTY   (-1)
#define BUFFER_OVERFLOW  1   // returned by buffer_push when oldest sample was overwritten

void buffer_init(void);
int  buffer_push(int data);       // returns BUFFER_OK or BUFFER_OVERFLOW
int  buffer_pop(int *data);       // returns BUFFER_OK or BUFFER_EMPTY
int  buffer_get_count(void);
int  buffer_get_overflow_count(void);

#endif
