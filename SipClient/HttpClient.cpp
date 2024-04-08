#include "pch.h"
#include "HttpClient.h"
#include <cpr/cpr.h>

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
			{"app",channel_info->App},
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
		SPDLOG_INFO("返回: {}", res.text);
		return true;
	}
	return false;
}


bool HttpClient::StopSendRtp(const std::string& app, const std::string& stream) {
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/stopSendRtp" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"app",app},
			{"stream", stream}
		},
		cpr::Timeout{ 3s }
	);

	if (res.status_code == 200)
	{
		SPDLOG_INFO("返回: {}", res.text);
		return true;
	}
	return false;
}


bool HttpClient::StartSendPlaybackRtp(
	std::shared_ptr<ChannelInfo> channel_info, std::string ssrc, std::string dst_ip,
	int dst_port, int local_port, std::string& start_time, std::string& end_time, bool use_tcp)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/startSendPlaybackRtp" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"app",channel_info->App},
			{"stream",channel_info->Stream},
			{"ssrc",ssrc},
			{"dst_url",dst_ip},
			{"dst_port",std::to_string(dst_port)},
			{"src_port",std::to_string(local_port)},
			{"start_time",start_time},
			{"end_time",end_time},
			{"is_udp",use_tcp ? "0" : "1"}
		},
		cpr::Timeout{ 3s }
	);

	if (res.status_code == 200)
	{
		SPDLOG_INFO("返回: {}", res.text);
		return true;
	}
	return false;
}


bool HttpClient::GetMp4RecordInfo(std::string stream,
	std::string start_time, std::string end_time, std::string& response)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/getMp4RecordInfo" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"stream",stream},
			{"start_time",start_time},
			{"end_time",end_time}
		},
		cpr::Timeout{ 3s }
	);

	response = res.text;
	if (res.status_code == 200)
	{
		return true;
	}
	return false;
}


bool HttpClient::SetPause(std::string app, std::string stream, bool pause)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/action" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"app",app},
			{"stream",stream},
			{"pause",pause ? "true" : "false"}
		},
		cpr::Timeout{ 3s }
	);

	return true;
}


bool HttpClient::SetSpeed(std::string app, std::string stream, float speed)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/action" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"app",app},
			{"stream",stream},
			{"speed", std::to_string(speed)}
		},
		cpr::Timeout{ 3s }
	);

	return true;
}
