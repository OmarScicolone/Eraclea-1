#include "pus.h"
#include "system.h"
#include <stdio.h>

// Forward declaration
void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len);

void handle_tc(pus_packet_t* pkt)
{
    if (!pkt) return;

    switch (pkt->header.service)
    {
        case 1: // Power / mode control
            switch (pkt->header.subtype)
            {
                case 1: // Activate (IDLE → ACTIVE)
                    printf("[TC] ACTIVATE command received\n");
                    system_activate();
                    send_tm(1, 1, NULL, 0); // ACK
                    break;

                case 2: // Start Downlink (ACTIVE → DOWNLINK)
                    printf("[TC] START DOWNLINK command received\n");
                    system_start_downlink();
                    send_tm(1, 2, NULL, 0); // ACK
                    break;

                case 3: // Shutdown (internal)
                    printf("[TC] SHUTDOWN command received\n");
                    system_shutdown();
                    send_tm(1, 3, NULL, 0); // ACK
                    break;

                case 4: // Stop Downlink (DOWNLINK → ACTIVE)
                    printf("[TC] STOP DOWNLINK command received\n");
                    system_stop_downlink();
                    send_tm(1, 4, NULL, 0); // ACK
                    break;

                case 5: // Deactivate (ACTIVE → IDLE)
                    printf("[TC] DEACTIVATE command received\n");
                    system_deactivate();
                    send_tm(1, 5, NULL, 0); // ACK
                    break;

                default:
                    printf("[TC] Unknown power control subtype\n");
                    send_tm(1, 8, NULL, 0); // NACK
                    break;
            }
            break;

        case 17: // Connection test
            printf("[TC] Test command received\n");
            send_tm(1, 1, NULL, 0); // ACK
            break;

        case 3: // Housekeeping request
        {
            printf("[TC] HK request received\n");
            uint8_t data[2];
            data[0] = 42; // dummy status
            data[1] = 0;
            send_tm(3, 25, data, 2);
            break;
        }

        default:
            printf("[TC] Unknown service\n");
            send_tm(1, 8, NULL, 0); // NACK
            break;
    }
}
