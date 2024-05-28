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

	SPDLOG_WARN("--------------: {}", res.url.str());
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
	SPDLOG_WARN("--------------: {}", res.url.str());

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
	SPDLOG_WARN("--------------: {}", res.url.str());

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
	SPDLOG_WARN("--------------: {}", res.url.str());

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
	SPDLOG_WARN("--------------: {}", res.url.str());

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
	SPDLOG_WARN("--------------: {}", res.url.str());

	return true;
}

std::vector<media::mediaserver_stream_item> HttpClient::GetMediaList()
{
	std::vector<media::mediaserver_stream_item> items;
	try
	{
		cpr::Response res = cpr::Get(
			cpr::Url{ _base_url,"/index/api/getMediaList" },
			cpr::Parameters{
				{"secret",_server_info->Secret},
				{"vhost","__defaultVhost__"},
			},
			cpr::Timeout{ 2s }
		);

		SPDLOG_WARN("--------------: {}", res.url.str());
		if (res.status_code == 200 && !res.text.empty())
		{
			nlohmann::json doc = nlohmann::json::parse(res.text);
			if (!doc.empty())
			{
				auto code = doc.at("code").get<int>();
				if (code == 0 && doc.find("data") != doc.end())
				{
					auto& datas = doc.at("data");
					int i = 0;
					for (auto&& item : datas)
					{
						auto p = item.get<media::mediaserver_stream_item>();
						items.push_back(p);
					}
				}
			}
		}
	}
	catch (std::exception& e)
	{
		SPDLOG_ERROR(e.what());
	}
}

bool HttpClient::AddDistributeStream(std::shared_ptr<DistributeItem> item)
{
	cpr::Response res = cpr::Get(
		cpr::Url{ _base_url,"/index/api/getMp4RecordInfo" },
		cpr::Parameters{
			{"secret",_server_info->Secret},
			{"vhost","__defaultVhost__"},
			{"app",item->App},
			{"stream",item->Stream},
			{"url",item->Source},
			{"retry_count",std::to_string(item->RetryTimes)},
			{"enable_mp4",item->RecordMP4 ? "true" : "false"},
			{"mp4_as_player","true"},
			{"auto_close","false"}
		},
		cpr::Timeout{ 3s }
	);
	SPDLOG_WARN("--------------: {}", res.url.str());

	auto response = res.text;
	if (res.status_code == 200)
	{
		return true;
	}
	return false;
}


void media::to_json(nlohmann::json& j, const mediaserver_stream_item& p)
{
}

void media::from_json(const nlohmann::json& j, mediaserver_stream_item& p)
{
	j.at("aliveSecond").get_to(p.aliveSecond);
	j.at("app").get_to(p.app);
	j.at("bytesSpeed").get_to(p.bytesSpeed);
	j.at("createStamp").get_to(p.createStamp);
	j.at("originUrl").get_to(p.originUrl);
	j.at("readerCount").get_to(p.readerCount);
	j.at("originType").get_to(p.originType);
	j.at("originTypeStr").get_to(p.originTypeStr);
	j.at("schema").get_to(p.schema);
	j.at("stream").get_to(p.stream);
	j.at("vhost").get_to(p.vhost);
	j.at("isRecordingHLS").get_to(p.isRecordingHLS);
	j.at("isRecordingMP4").get_to(p.isRecordingMP4);

	auto& tracks = j.at("tracks");
	for (auto&& track : tracks)
	{
		if (track.at("codec_type") == 0)
		{
			track.at("codec_id").get_to(p.codec_id);
			track.at("codec_id_name").get_to(p.codec_id_name);
			track.at("fps").get_to(p.fps);
			track.at("height").get_to(p.height);
			track.at("width").get_to(p.width);
		}
	}

	p.resolution = fmt::format("{}*{}", p.width, p.height);
	p.path = fmt::format("{}/{}/{}", p.vhost, p.app, p.stream);
}
