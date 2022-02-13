#pragma once
#include "memory.h"

constexpr addr_t interruptTable = 0x0;

// up to 40h
enum InterruptTable
{
	HALT = 0x0,
	DE,
	VIDEOCONTROLLER = 0x10,
	VBI
};

inline bool isHalted = false;
void interrupt(uint8_t interrupt);

// you should use it from devicecontroller thread, lock it otherwise.
void addDelayedInterrupt(uint8_t interrupt);
void handleDelayedInterrupts(void);
