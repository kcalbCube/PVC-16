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
	static void addOperation(Operation);
	static std::deque<Operation>& getOperations(void);
	std::deque<std::function<void()>> workList, emptyList;
	std::vector<Device*> devices;
	std::unique_ptr<std::jthread> thread;
	void start(void);
	void addDevice(Device* device);

	~DeviceController(void);
};

class Device
{
public:
	DeviceController* dc = nullptr;
	unsigned tick = 0;

	virtual void process(void) = 0;
	virtual ~Device(void) = default;
};

class DebugOutputDevice : public Device
{
	void process(void) override;
};