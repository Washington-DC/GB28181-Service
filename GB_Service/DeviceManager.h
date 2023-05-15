#pragma once
#include "Device.h"

class DeviceManager
{
public:
	SINGLETON_DEFINE(DeviceManager);

	void AddDevice(Device::Ptr device);
	Device::Ptr GetDevice(const std::string& device_id);
	void RemoveDevice(const std::string& device_id);

	std::vector<Device::Ptr> GetDeviceList();
	int GetDeviceCount();
	void UpdateDeviceStatus(const std::string& device_id, int status);
	void UpdateDeviceLastTime(const std::string& device_id, int64_t time);
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

