#include "interrupt.h"
#include "device.h"
#include "stack.h"

namespace interrupts
{
	void interrupt(const uint8_t interrupt)
	{
		if (!registers::status.interrupt)
			return;

		switch (interrupt)
		{
		case HALT:
			isHalted = true;
			break;

		case VIDEOCONTROLLER:
			switch (read(registers::AL))
			{
			case 1:
				dc->operations.push_back(Operation::VIDEOCONTROLLER_SET_MODE);
				break;
			}
			break;

		default:
		{
			const auto addr = mc.read16(interrupt * 2);
			if (!addr)
				break;
			stack::push(registers::IP);
			stack::pushf();
			write(registers::IP, addr);
		}
		break;
		}
	}

	std::deque<uint8_t> delayedInterrupts;

	void delayedInterrupt(uint8_t interrupt)
	{
		if (registers::status.interrupt)
			delayedInterrupts.push_back(interrupt);
	}

	void handleDelayedInterrupts(void)
	{
		if (!registers::status.interrupt)
			delayedInterrupts.clear();
		else if (!delayedInterrupts.empty())
		{
			interrupt(delayedInterrupts[0]);
			delayedInterrupts.pop_front();
		}
	}
}