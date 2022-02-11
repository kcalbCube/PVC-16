#pragma once
#include <deque>
#include <chrono>
#include <thread>
#include <future>
#include <functional>
#include <vector>

class Device;
class DeviceController
{
public:
	std::deque<std::function<void()>> workList, emptyList;
	//std::chrono::duration lag = std::chrono::milliseconds(1);
	std::vector<Device*> devices;
	void start(void);
	void addDevice(Device* device);
};

class Device
{
public:
	DeviceController* dc = nullptr;

	virtual void process(void) = 0;
};

class DebugOutputDevice : public Device
{
	void process(void);
};