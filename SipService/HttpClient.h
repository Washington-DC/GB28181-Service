#pragma once
#include "Structs.h"

class HttpClient
{
public:
	SINGLETON_DEFINE(HttpClient);

	void Init(std::shared_ptr<MediaServerInfo> server_info);

	bool StartSendRtp(std::shared_ptr<ChannelInfo> info, std::string ssrc, std::string dst_ip, int dst_port,int local_port, bool use_tcp = false);
	bool StopSendRtp(std::shared_ptr<ChannelInfo> info);

private:
	std::shared_ptr<MediaServerInfo> _server_info = nullptr;
	std::wstring _base_url = L"http://127.0.0.1:8000";;
private:
	HttpClient() = default;
};