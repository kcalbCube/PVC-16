#pragma once
#include "memory.h"

constexpr addr_t interruptTable = 0x0;

// up to 40h
enum InterruptTable
{
	HALT = 0x0,
	R1, R2, R3, R4,
	DOUT
};

inline bool isHalted = false;
void interrupt(uint8_t interrupt);
