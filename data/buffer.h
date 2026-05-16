#ifndef BUFFER_H
#define BUFFER_H

#define BUFFER_SIZE 16

#define BUFFER_OK 0
#define BUFFER_EMPTY (-1)
#define BUFFER_OVERFLOW 1

void buffer_init(void);
int buffer_push(int data);
int buffer_pop(int *data);
int buffer_get_count(void);
int buffer_get_overflow_count(void);
void buffer_reset_overflow(void);

#endif