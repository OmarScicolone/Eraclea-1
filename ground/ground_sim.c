#include "pus.h"
#include "tc_handler.h"

void simulate_ground(void)
{
    pus_packet_t pkt;

    pkt.header.version = 1;
    pkt.header.type = 1; // TC
    pkt.header.service = 17;
    pkt.header.subtype = 1;
    pkt.header.length = 0;

    handle_tc(&pkt);
}
