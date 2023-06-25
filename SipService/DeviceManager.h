#pragma once
#include "Device.h"

class DeviceManager
{
public:
	SINGLETON_DEFINE(DeviceManager);

	void Init();
	void AddDevice(Device::Ptr device);
	Device::Ptr GetDevice(const std::string& device_id);
	Device::Ptr GetDevice(const std::string& ip, const std::string& port);
	void RemoveDevice(const std::string& device_id);

	std::vector<Device::Ptr> GetDeviceList();
	int GetDeviceCount();
	void UpdateDeviceStatus(const std::string& device_id, int status);
	void UpdateDeviceLastTime(const std::string& device_id, time_t time = time(nullptr));
	void UpdateDeviceChannelCount(const std::string& device_id, int count);

	void Start();

private:
	DeviceManager() = default;
	void CheckDeviceStatus();

private:

	std::unordered_map<std::string, Device::Ptr> _devices;
	toolkit::Timer::Ptr _check_timer = nullptr;
	std::mutex _mutex;
};

