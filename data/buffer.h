#ifndef BUFFER_H
#define BUFFER_H

#define BUFFER_SIZE 16

void buffer_init(void);
int buffer_push(int data);
int buffer_pop(int *data);

#endif