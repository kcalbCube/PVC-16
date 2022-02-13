#pragma once
#include <deque>
#include <chrono>
#include <thread>
#include <future>
#include <functional>
#include <vector>


enum class Operation
{
	VIDEOCONTROLLER_SET_MODE
};

class Device;
class DeviceController
{
public:
	std::deque<Operation> operations;
	std::deque<std::function<void()>> workList, emptyList;
	//std::chrono::duration lag = std::chrono::milliseconds(1);
	std::vector<Device*> devices;
	void start(void);
	void addDevice(Device* device);
};

inline DeviceController* dc = nullptr;

class Device
{
public:
	DeviceController* dc = nullptr;
	unsigned tick = 0;

	virtual void process(void) = 0;
};

class DebugOutputDevice : public Device
{
	void process(void);
};