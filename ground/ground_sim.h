#ifndef GROUND_SIM_H
#define GROUND_SIM_H

#include "pus.h"

void simulate_ground_tc(void);
void print_tm_output(uint8_t service, uint8_t subtype, uint8_t* data, uint16_t len);

#endif
