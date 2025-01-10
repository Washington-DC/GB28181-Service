#include "pch.h"
#include "ZlmServer.h"
#include "DTO.h"
#include "ConfigManager.h"
#include <cpr/cpr.h>

#ifdef _WIN32
#ifdef _DEBUG
#pragma comment(lib,"libcurl-d_imp.lib")
#pragma comment(lib,"cpr-d.lib")
#else
#pragma comment(lib,"libcurl_imp.lib")
#pragma comment(lib,"cpr.lib")
#endif
#endif

void ZlmServer::Init(std::shared_ptr<MediaServerInfo> info)
{
	_info = info;
	_base_url = fmt::format("http://{}:{}", _info->IP, _info->Port);
}


void ZlmServer::UpdateHeartbeatTime(time_t t)
{
	std::scoped_lock<std::mutex> g(_mutex);
	_last_heartbeat_time = t;

	if (_delay_task)
		_delay_task->cancel();

	_delay_task = toolkit::EventPollerPool::Instance().getPoller()->doDelayTask(1000 * 10, [this]()
		{
			auto now = time(nullptr);
			if (now - _last_heartbeat_time > 6)
				UpdateStatus(false);
			else
				UpdateStatus(true);
			return 0;
		});
}


void ZlmServer::UpdateStatus(bool flag)
{
	std::scoped_lock<std::mutex> g(_mutex);
	_connected = true;
}


bool ZlmServer::IsConnected()
{
	return _connected;
}


//单端口模式时，使用固定的端口接收rtp数据，通过SSRC区分不同的流
//多端口模式时，调用OpenRtpServer，创建RTP数据接收端口，端口由zlm分配，通过设备和通道ID标识不同的流
int ZlmServer::OpenRtpServer(const std::string& stream_id)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/openRtpServer" },
		cpr::Parameters{
			{"secret",_info->Secret},
			{"port","0"},
			{"tcp_mode", "0"},
			{"enable_tcp","0"}, // 兼容旧版本ZLM
			{"stream_id",stream_id}
		},
		cpr::Timeout{ 3s }
	);

	if (res.status_code == 200)
	{
		auto info = nlohmann::json::parse(res.text).get<dto::ResponseInfo>();
		if (info.Code == 0)
		{
			return info.Port;
		}
	}
	return -1;
}


void ZlmServer::CloseRtpServer(const std::string& stream_id)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/closeRtpServer" },
		cpr::Parameters{
			{"secret",_info->Secret},
			{"stream_id",stream_id}
		},
		cpr::Timeout{ 3s }
	);

	if (res.status_code == 200)
	{

	}
}


std::string ZlmServer::ListRtpServer()
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/listRtpServer" },
		cpr::Parameters{ {"secret",_info->Secret} },
		cpr::Timeout{ 3s }
	);

	if (res.status_code == 200)
	{
		return res.text;
	}

	return "";
}

bool ZlmServer::SinglePortMode()
{
	return _info->SinglePortMode;
}

int ZlmServer::FixedRtpPort()
{
	return _info->SinglePortMode ? _info->RtpPort : -1;
}

int ZlmServer::MaxPlayWaitTime()
{
	return _info->PlayWait;
}
