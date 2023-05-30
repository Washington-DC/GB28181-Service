#include "pch.h"
#include "HttpClient.h"

#include <cpr/cpr.h>

#ifdef _DEBUG
#pragma comment(lib,"libcurl-d_imp.lib")
#pragma comment(lib,"cpr-d.lib")
#else
#pragma comment(lib,"libcurl_imp.lib")
#pragma comment(lib,"cpr.lib")
#endif

void HttpClient::Init(std::shared_ptr<MediaServerInfo> info) {
	this->_server_info = info;
	_base_url = fmt::format("http://{}:{}", _server_info->IP, _server_info->Port);
}

bool HttpClient::StartSendRtp(
	std::shared_ptr<ChannelInfo> channel_info, std::string ssrc,
	std::string dst_ip, int dst_port, int local_port, bool use_tcp)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/startSendRtp" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"app", channel_info->App},
			{"stream",channel_info->Stream},
			{"ssrc",ssrc},
			{"dst_url",dst_ip},
			{"dst_port",std::to_string(dst_port)},
			{"src_port",std::to_string(local_port)},
			{"is_udp",use_tcp ? "0" : "1"}
		},
		cpr::Timeout{ 3s }
	);

	if (res.status_code == 200)
	{
		LOG(INFO) << "·µ»Ø: " << res.text;
		return true;
	}
	return false;
}

bool HttpClient::StopSendRtp(std::shared_ptr<ChannelInfo> channel_info) {
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/stopSendRtp" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"app", channel_info->App},
			{"stream",channel_info->Stream}
		},
		cpr::Timeout{ 3s }
	);

	return true;
}