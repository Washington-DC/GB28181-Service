#include "pch.h"
#include "DeviceManager.h"

void DeviceManager::AddDevice(Device::Ptr device)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	_devices[device->GetDeviceID()] = device;
}

Device::Ptr DeviceManager::GetDevice(const std::string& device_id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _devices.find(device_id);
	if (iter != _devices.end())
	{
		return iter->second;
	}
	return nullptr;
}

Device::Ptr DeviceManager::GetDevice(const std::string& ip, const std::string& port)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	for (auto&& dev : _devices)
	{
		if (dev.second->GetIP() == ip && dev.second->GetPort() == port)
		{
			return dev.second;
		}
	}
	return nullptr;
}

void DeviceManager::RemoveDevice(const std::string& device_id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	_devices.erase(device_id);
}

std::vector<Device::Ptr> DeviceManager::GetDeviceList()
{
	std::scoped_lock<std::mutex> lk(_mutex);
	std::vector<Device::Ptr> devices;
	for (auto&& dev : _devices)
	{
		devices.push_back(dev.second);
	}
	return devices;
}

int DeviceManager::GetDeviceCount()
{
	std::scoped_lock<std::mutex> lk(_mutex);
	return (int)_devices.size();
}

void DeviceManager::UpdateDeviceStatus(const std::string& device_id, int status)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _devices.find(device_id);
	if (iter != _devices.end())
	{
		iter->second->SetStatus(status);
	}
}

void DeviceManager::UpdateDeviceLastTime(const std::string& device_id, time_t time)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _devices.find(device_id);
	if (iter != _devices.end())
	{
		iter->second->UpdateLastTime(time);
	}
}

void DeviceManager::UpdateDeviceChannelCount(const std::string& device_id, int count)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _devices.find(device_id);
	if (iter != _devices.end())
	{
		iter->second->SetChannelCount(count);
	}
}

void DeviceManager::Start()
{
	CheckDeviceStatus();
}

void DeviceManager::CheckDeviceStatus()
{
	_check_timer.reset(new toolkit::Timer(
		3 * 60,
		[this]()
		{
			std::scoped_lock<std::mutex> lk(_mutex);

			//TODO£º ÐÄÌø³¬Ê±ÅÐ¶Ï
			for (auto&& dev : _devices)
			{
				
			}

			return true;
		},
		nullptr)
	);
}
