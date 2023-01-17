#pragma once
#include "pch.h"

struct SipServerInfo
{
	std::string IP;
	int Port;
	std::string ID;
	std::string Password;
};

struct MediaServerInfo
{
	std::string IP;
	int Port;
	std::string Secret;
};

struct ChannelInfo
{
	std::string ID;
	std::string Name;
	std::string App;
	std::string Stream;
};

struct DeviceInfo
{
	std::string IP;
	int Port;
	std::string ID;
	IPPROTO Protocol;
	std::string Name;
	std::string Manufacturer;
	int HeartbeatInterval;

	std::vector<std::shared_ptr<ChannelInfo>> Channels;
};


