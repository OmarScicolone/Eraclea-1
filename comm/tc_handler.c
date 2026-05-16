#include "pus.h"
#include "system.h"
#include <stdio.h>

// forward
void send_tm(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len);

void handle_tc(pus_packet_t* pkt)
{
    if (!pkt) return;

    switch (pkt->header.service)
    {
        case 1: // Power control
            switch (pkt->header.subtype)
            {
                case 1: // Switch ON
                    printf("[TC] SWITCH ON command received\n");
                    system_power_on();
                    send_tm(1, 1, NULL, 0); // ACK
                    break;

                case 2: // Enable Data
                    printf("[TC] ENABLE DATA command received\n");
                    system_enable_data();
                    send_tm(1, 2, NULL, 0); // ACK
                    break;

                case 3: // Shutdown
                    printf("[TC] SHUTDOWN command received\n");
                    system_shutdown();
                    send_tm(1, 3, NULL, 0); // ACK
                    break;

                case 4: // Disable Data
                    printf("[TC] DISABLE DATA command received\n");
                    system_disable_data();
                    send_tm(1, 4, NULL, 0); // ACK
                    break;

                case 5: // Enter Idle
                    printf("[TC] ENTER IDLE command received\n");
                    system_in_idle();
                    send_tm(1, 5, NULL, 0); // ACK
                    break;

                default:
                    printf("[TC] Unknown power control subtype\n");
                    send_tm(1, 8, NULL, 0); // NACK
                    break;
            }
            break;

        case 17: // test
            printf("[TC] Test command received\n");
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
            send_tm(1, 8, NULL, 0); // NACK
            break;
    }
}
