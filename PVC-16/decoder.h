#pragma once
#include <cstdint>

#include "opcode.h"

void process(void);

uint16_t readAddress(SIB sib, uint16_t disp);