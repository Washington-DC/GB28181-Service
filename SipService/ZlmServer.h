#pragma once
#include "Structs.h"
#include "SSRC_Config.h"

class ZlmServer
{
public:
	SINGLETON_DEFINE(ZlmServer);

	void Init(std::shared_ptr<MediaServerInfo> info);

	int OpenRtpServer(const std::string& stream_id);
	void CloseRtpServer(const std::string& stream_id);
	std::string ListRtpServer();

	bool SinglePortMode();
	int FixedRtpPort();
	int MaxPlayWaitTime();

	void UpdateHeartbeatTime(time_t t = time(nullptr));
	void UpdateStatus(bool flag = true);
	bool IsConnected();

private:
	ZlmServer() = default;

	std::shared_ptr<MediaServerInfo> _info = nullptr;
	time_t _last_heartbeat_time = 0;
	bool _connected = false;
	std::mutex _mutex;
	std::string _base_url ;
	toolkit::EventPoller::DelayTask::Ptr _delay_task = nullptr;
};