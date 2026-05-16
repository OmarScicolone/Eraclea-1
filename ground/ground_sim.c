#include "ground_sim.h"
#include <stdio.h>

void print_tm_output(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len)
{
    printf("[GROUND OUTPUT] TM: Service=%d, Subtype=%d, Len=%d\n", service, subtype, len);
    
    if (data && len > 0) {
        printf("  Data: ");
        for (int i = 0; i < len; i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
    }
}
