#include "pch.h"
#include "Device.h"

void Channel::InsertSubChannel(const std::string parent_id, const std::string& channel_id, Channel::Ptr channel)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	if (parent_id == _channel_id)
	{
		_sub_channels[channel_id] = channel;
		_sub_channel_count++;
	}
	else
	{
		for (auto&& ch : _sub_channels)
		{
			ch.second->InsertSubChannel(parent_id, channel_id, channel);
		}
	}
}

Channel::Ptr Channel::GetSubChannel(const std::string& channel_id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _sub_channels.find(channel_id);
	if (iter != _sub_channels.end())
	{
		return iter->second;
	}
	return nullptr;
}

void Channel::DeleteSubChannel(const std::string& channel_id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _sub_channels.find(channel_id);
	if (iter != _sub_channels.end())
	{
		_sub_channels.erase(iter);
		_sub_channel_count--;
	}
	else
	{
		for (auto&& ch : _sub_channels)
		{
			ch.second->DeleteSubChannel(channel_id);
		}
	}
}

std::vector<Channel::Ptr> Channel::GetAllSubChannels()
{
	std::scoped_lock<std::mutex> lk(_mutex);
	std::vector<Channel::Ptr> channels;
	for (auto&& ch : _sub_channels)
	{
		channels.push_back(ch.second);
	}
	return channels;
}

int Channel::GetChannelCount()
{
	std::scoped_lock<std::mutex> lk(_mutex);
	return _sub_channel_count;
}

void Channel::AddChannelCount()
{
	std::scoped_lock<std::mutex> lk(_mutex);
	_sub_channel_count++;
}

void Channel::SubChannelCount()
{
	std::scoped_lock<std::mutex> lk(_mutex);
	_sub_channel_count--;
	if (_sub_channel_count < 0)
		_sub_channel_count = 0;
}

std::string Channel::toString()
{
	return std::string();
}

void Channel::SetParentID(const std::string& parent_id)
{
	_parent_id = parent_id;
}

std::string Channel::GetParentID() const
{
	return _parent_id;
}

void Channel::SetChannelID(const std::string& channel_id)
{
	_channel_id = channel_id;
}

std::string Channel::GetChannelID() const
{
	return _channel_id;
}

void Channel::SetName(const std::string& name)
{
	_name = name;
}

std::string Channel::GetName() const
{
	return _name;
}

void Channel::SetManufacturer(const std::string& manufacturer)
{
	_manufacturer = manufacturer;
}

std::string Channel::GetManufacturer() const
{
	return _manufacturer;
}

void Channel::SetModel(const std::string& model)
{
	_model = model;
}

std::string Channel::GetModel() const
{
	return _model;
}

void Channel::SetOwner(const std::string& owner)
{
	_owner = owner;
}

std::string Channel::GetOwner() const
{
	return _owner;
}

void Channel::SetCivilCode(const std::string& civil_code)
{
	_civil_code = civil_code;
}

std::string Channel::GetCivilCode() const
{
	return _civil_code;
}

void Channel::SetAddress(const std::string& address)
{
	_address = address;
}

std::string Channel::GetAddress() const
{
	return _address;
}

void Channel::SetParental(const std::string& parental)
{
	_parental = parental;
}

std::string Channel::GetParental() const
{
	return _parental;
}

void Channel::SetRegisterWay(const std::string& register_way)
{
	_register_way = register_way;
}

std::string Channel::GetRegisterWay() const
{
	return _register_way;
}

void Channel::SetSecrety(const std::string& secrety)
{
	_secrety = secrety;
}

std::string Channel::GetSecrety() const
{
	return _secrety;
}

void Channel::SetStreamNum(const std::string& stream_num)
{
	_stream_num = stream_num;
}

std::string Channel::GetStreamNum() const
{
	return _stream_num;
}

void Channel::SetIpAddress(const std::string& ip)
{
	_ip = ip;
}

std::string Channel::GetIpAddress() const
{
	return _ip;
}

void Channel::SetPtzType(const std::string& ptz_type)
{
	_ptz_type = ptz_type;
}

std::string Channel::GetPtzType() const
{
	return _ptz_type;
}

void Channel::SetDownloadSpeed(const std::string& speed)
{
	_download_speed = speed;
}

std::string Channel::GetDownloadSpeed() const
{
	return _download_speed;
}


//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------


Device::Device(const std::string& device_id, const std::string& ip, const std::string& port)
{
	_device_id = device_id;
	_ip = ip;
	_port = port;
}

void Device::InsertChannel(const std::string& parent_id, const std::string& channel_id, Channel::Ptr channel)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	if (parent_id == parent_id)
	{
		_channels[channel_id] = channel;
		_channel_count++;
	}
	else
	{
		for (auto&& ch : _channels)
		{
			ch.second->InsertSubChannel(parent_id, channel_id, channel);
		}
	}
}

void Device::DeleteChannel(const std::string& channel_id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _channels.find(channel_id);
	if (iter != _channels.end())
	{
		_channels.erase(iter); 
		_channel_count--;
	}
	else
	{
		for (auto&& ch : _channels)
		{
			ch.second->DeleteSubChannel(channel_id);
		}
	}
}

Channel::Ptr Device::GetChannel(const std::string& channel_id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _channels.find(channel_id);
	if (iter != _channels.end())
	{
		return iter->second;
	}
	return nullptr;
}

std::vector<Channel::Ptr> Device::GetAllChannels()
{
	std::scoped_lock<std::mutex> lk(_mutex);
	std::vector<Channel::Ptr> channels;
	for (auto&& ch : _channels)
	{
		channels.push_back(ch.second);
	}
	return channels;
}

std::string Device::GetDeviceID() const
{
	return _device_id;
}

void Device::SetDeviceID(const std::string& id)
{
	_device_id = id;
}

std::string Device::GetName() const
{
	return _name;
}

void Device::SetName(const std::string& name)
{
	_name = name;
}

std::string Device::GetIP() const
{
	return _ip;
}

void Device::SetIP(const std::string& ip)
{
	_ip = ip;
}

std::string Device::GetPort() const
{
	return _port;
}

void Device::SetPort(const std::string& port)
{
	_port = port;
}

std::string Device::GetTransport() const
{
	return _transport;
}

void Device::SetTransport(const std::string& transport)
{
	_transport = transport;
}

int Device::GetStatus() const
{
	return _status;
}

void Device::SetStatus(int status)
{
	_status = status;
}

int64_t Device::GetRegistTime()
{
	return _regist_time;
}

void Device::SetRegistTime(int64_t t)
{
	_regist_time = t;
}

int64_t Device::GetLastTime()
{
	return _last_time;
}

void Device::SetLastTime(int64_t t)
{
	_last_time = t;
}

int Device::GetChannelCount()
{
	return _channel_count;
}

void Device::SetChannelCount(int count)
{
	_channel_count = count;
}

std::string Device::GetParentID() const
{
	return _parent_id;
}

void Device::SetParentID(const std::string& parent_id)
{
	_parent_id = parent_id;
}

std::string Device::toString()
{
	return std::string();
}
