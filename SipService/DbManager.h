#pragma once
#include <sqlite3pp.h>
#include "Device.h"

class DbManager
{
public:
	SINGLETON_DEFINE(DbManager);

	bool Init(const std::string& db_path);
	bool AddOrUpdateDevice(Device::Ptr device,bool update = false);
	bool AddOrUpdateChannel(const std::string& device_id,Channel::Ptr channel, bool update = false);

	std::vector<Device::Ptr> GetDeviceList();
	std::vector<Channel::Ptr> GetChannelList(const std::string& device_id);

private:
	bool _create_tables();
	std::shared_ptr<sqlite3pp::database> _db = nullptr;
	DbManager() = default;
	bool _checked = false;
};

