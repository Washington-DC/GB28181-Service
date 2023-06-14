#pragma once


class Channel
{
public:
	typedef std::shared_ptr<Channel> Ptr;

	Channel() = default;
	void InsertSubChannel(const std::string parent_id, const std::string& channel_id, Channel::Ptr channel);
	Channel::Ptr GetSubChannel(const std::string& channel_id);
	void DeleteSubChannel(const std::string& channel_id);
	std::vector<Channel::Ptr> GetAllSubChannels();

	int GetChannelCount();
	void AddChannelCount();
	void SubChannelCount();


	void SetParentID(const std::string& parent_id);
	std::string GetParentID() const;

	void SetChannelID(const std::string& channel_id);
	std::string GetChannelID() const;

	void SetName(const std::string& name);
	std::string GetName() const;

	void SetManufacturer(const std::string& manufacturer);
	std::string GetManufacturer() const;

	void SetModel(const std::string& model);
	std::string GetModel() const;

	void SetOwner(const std::string& owner);
	std::string GetOwner() const;

	void SetCivilCode(const std::string& civil_code);
	std::string GetCivilCode() const;

	void SetAddress(const std::string& address);
	std::string GetAddress() const;

	void SetStatus(const std::string& status);
	std::string GetStatus() const;

	void SetParental(const std::string& parental);
	std::string GetParental() const;

	void SetRegisterWay(const std::string& register_way);
	std::string GetRegisterWay() const;

	void SetSecrety(const std::string& secrety);
	std::string GetSecrety() const;

	void SetStreamNum(const std::string& stream_num);
	std::string GetStreamNum() const;

	void SetIpAddress(const std::string& ip);
	std::string GetIpAddress() const;

	void SetPtzType(const std::string& ptz_type);
	std::string GetPtzType() const;

	void SetDownloadSpeed(const std::string& speed);
	std::string GetDownloadSpeed() const;

	std::string toString();
	nlohmann::json toJson();

private:
	std::string _channel_id;
	std::string _parent_id;
	std::string _name;
	std::string _manufacturer;
	std::string _model;
	std::string _owner;
	std::string _civil_code;
	std::string _address;
	std::string _parental;
	std::string _register_way;
	std::string _secrety;
	std::string _stream_num;
	std::string _ip;
	std::string _status;

	int _sub_channel_count = 0;
	std::string _ptz_type;
	std::string _download_speed;

	std::map<std::string, Channel::Ptr> _sub_channels;

private:
	std::mutex _mutex;
};

class Device
{
public:
	typedef std::shared_ptr<Device> Ptr;

	Device() = default;
	Device(const std::string& device_id, const std::string& ip, const std::string& port);

	void InsertChannel(const std::string& parent_id, const std::string& channel_id, Channel::Ptr channel);
	void DeleteChannel(const std::string& channel_id);

	Channel::Ptr GetChannel(const std::string& channel_id);
	std::vector<Channel::Ptr> GetAllChannels();

	std::string GetDeviceID() const;
	void SetDeviceID(const std::string& id);

	std::string GetName() const;
	void SetName(const std::string& name);

	std::string GetIP() const;
	void SetIP(const std::string& ip);

	std::string GetPort() const;
	void SetPort(const std::string& port);

	std::string GetTransport() const;
	void SetTransport(const std::string& transport);

	void SetManufacturer(const std::string& manufacturer);
	std::string GetManufacturer() const;

	void SetModel(const std::string& model);
	std::string GetModel() const;

	int GetStatus() const;
	void SetStatus(int status);

	time_t GetRegistTime();
	void UpdateRegistTime(time_t t = time(nullptr));

	time_t GetLastTime();
	void UpdateLastTime(time_t t = time(nullptr));

	int GetChannelCount();
	void SetChannelCount(int count);

	std::string GetParentID() const;
	void SetParentID(const std::string& parent_id);

	std::string GetStreamIP() const;
	void SetStreamIP(const std::string& stream_ip);

	nlohmann::json toJson();
	std::string toString();

private:

	std::string _device_id;
	std::string _name;
	std::string _ip;
	std::string _port;
	std::string _transport = "UDP";
	std::string _manufacturer;
	std::string _model;
	int _status = 0;

	std::string _stream_ip;//ÊÕÁ÷IP

	time_t _regist_time = 0;
	time_t _last_time = 0;

	int _channel_count = 0;
	std::string _parent_id;

	std::map<std::string, Channel::Ptr> _channels;

	std::mutex _mutex;
};

