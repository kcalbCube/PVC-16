#include "interrupt.h"
#include "device.h"
#include "stack.h"
#include <queue>

namespace interrupts
{
	bool isHalted(void)
	{
		return isHaltedi;
	}

	void interrupt(const uint8_t interrupt)
	{
		if (!registers::status.interrupt)
			return;

		switch (interrupt)
		{
		case HALT:
			isHaltedi = true;
			break;

		case VIDEOCONTROLLER:
			switch (read(registers::AL))
			{
			case 1:
				DeviceController::addOperation(Operation::VIDEOCONTROLLER_SET_MODE);
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

	std::queue<uint8_t> delayedInterrupts;

	void delayedInterrupt(uint8_t interrupt)
	{
		if (registers::status.interrupt)
			delayedInterrupts.emplace(interrupt);
	}

	void handleDelayedInterrupts(void)
	{
		if (!delayedInterrupts.empty()) [[unlikely]]
		{
			interrupt(delayedInterrupts.front());
			delayedInterrupts.pop();
		}
	}
}
