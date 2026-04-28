#include "pus.h"
#include <stdio.h>

void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len)
{
    pus_packet_t pkt;

    pkt.header.version = 1;
    pkt.header.type = 0; // TM
    pkt.header.service = service;
    pkt.header.subtype = subtype;
    pkt.header.length = len;

    printf("[TM] Service=%d Subtype=%d Len=%d\n",
           service, subtype, len);

    if (data && len > 0)
    {
        for (int i = 0; i < len; i++)
            printf(" %d", data[i]);
        printf("\n");
    }
}