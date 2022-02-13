#include "interrupt.h"
#include "device.h"
#include <iostream>
#include "stack.h"

void interrupt(const uint8_t interrupt)
{
	switch(interrupt)
	{
	case HALT:
		isHalted = true;
		break;

	case VIDEOCONTROLLER:
		switch (readRegister(AL))
		{
		case 1:
			dc->operations.push_back(Operation::VIDEOCONTROLLER_SET_MODE);
			break;
		}
		break;

	default:
		StackController::push(IP);
		writeRegister(IP, mc.read16(interrupt * 2));
		break;
	}
}

std::deque<uint8_t> delayedInterrupts;

void addDelayedInterrupt(uint8_t interrupt)
{
	delayedInterrupts.push_back(interrupt);
}

void handleDelayedInterrupts(void)
{
	if (!delayedInterrupts.empty())
	{
		interrupt(delayedInterrupts[0]);
		delayedInterrupts.pop_back();
	}
}