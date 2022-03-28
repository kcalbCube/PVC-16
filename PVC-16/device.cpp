#include "device.h"
#include "bus.h"
#include <iostream>

namespace
{
	std::deque<Operation> operations;
};

void DeviceController::addOperation(Operation op)
{
	operations.push_back(op);
}

std::deque<Operation>& DeviceController::getOperations(void)
{
	return operations;
}

void DeviceController::start(void)
{
	thread = std::make_unique<std::jthread>([this](std::stop_token stopToken) mutable -> void
	{
		while (!stopToken.stop_requested())
		{
			auto start = std::chrono::high_resolution_clock::now();
			for (Device* device : this->devices)
			{
				++device->tick;
				device->process();
			}
			operations.clear();
			std::this_thread::sleep_until(start + std::chrono::milliseconds(1));
		}
	});

	thread->detach();
}

DeviceController::~DeviceController(void)
{
	thread->request_stop();
}
void DeviceController::addDevice(Device* device)
{
	device->dc = this;
	devices.push_back(device);
}

void DebugOutputDevice::process(void)
{
	if (auto ch = busRead(BUS_DOUT); ch)
	{
		putchar(ch);
		busWrite(BUS_DOUT, 0);
	}
}