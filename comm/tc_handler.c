#include "pus.h"
#include <stdio.h>

// forward
void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len);

void handle_tc(pus_packet_t* pkt)
{
    if (!pkt) return;

    switch (pkt->header.service)
    {
        case 17: // test
            printf("[TC] Test command received\n");

            // risposta ACK
            send_tm(1, 1, NULL, 0);
            break;

        case 3: // richiesta housekeeping
        {
            printf("[TC] HK request\n");

            uint8_t data[2];
            data[0] = 42; // dummy status
            data[1] = 0;

            send_tm(3, 25, data, 2);
            break;
        }

        default:
            printf("[TC] Unknown service\n");
            send_tm(1, 2, NULL, 0); // NACK
            break;
    }
}
