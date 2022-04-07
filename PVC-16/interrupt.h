#pragma once
#include "memory.h"

namespace interrupts
{
	constexpr addr_t interruptTable = 0x0;

	// up to 40h
	enum InterruptTable
	{
		HALT = 0x0,
		DE,
		VIDEOCONTROLLER = 0x10,
		VBI
	};

	bool isHalted(void);

	inline bool isHaltedi = false;
	void interrupt(uint8_t interrupt);

	// you should use it from devicecontroller thread, lock it otherwise.
	void delayedInterrupt(uint8_t interrupt);
	void handleDelayedInterrupts(void);
}
