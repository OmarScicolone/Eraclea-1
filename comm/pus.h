#ifndef PUS_H
#define PUS_H

#include <stdint.h>

typedef struct {
    uint8_t version;
    uint8_t type;      // 0 = TM, 1 = TC
    uint8_t service;
    uint8_t subtype;
    uint16_t length;
} pus_header_t;

typedef struct {
    pus_header_t header;
    uint8_t data[32];
} pus_packet_t;

#endif