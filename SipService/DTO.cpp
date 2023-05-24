#include "pch.h"
#include "DTO.h"

namespace dto
{
	void from_json(const nlohmann::json& j, ZlmStreamInfo& info)
	{
		j.at("mediaServerId").get_to(info.MediaServerID);
		j.at("app").get_to(info.App);
		j.at("stream").get_to(info.Stream);
		j.at("schema").get_to(info.Schema);
		j.at("vhost").get_to(info.Vhost);
		if (j.contains("regist"))
		{
			j.at("regist").get_to(info.Regist);

			if (j.contains("originType"))
			{
				j.at("originType").get_to(info.OriginType);
			}
		}
		
		if (j.contains("port"))
		{
			j.at("port").get_to(info.Port);
		}
		if (j.contains("ip"))
		{
			j.at("ip").get_to(info.IP);
		}
		if (j.contains("params"))
		{
			j.at("params").get_to(info.Params);
		}
	}

	void to_json(nlohmann::json& j, const ZlmStreamInfo& p)
	{

	}

	void from_json(const nlohmann::json& j, ResponseInfo& info)
	{
		j.at("code").get_to(info.Code);
		j.at("port").get_to(info.Port);
	}

	void to_json(nlohmann::json& j, const ResponseInfo& p)
	{

	}

	void from_json(const nlohmann::json& j, RtpServerInfo& info)
	{
		j.at("mediaServerId").get_to(info.MediaServerID);
		j.at("local_port").get_to(info.LocalPort);
		j.at("re_use_port").get_to(info.ReusePort);
		j.at("ssrc").get_to(info.SSRC);
		j.at("stream_id").get_to(info.StreamID);
		j.at("TcpMode").get_to(info.TcpMode);
	}

	void to_json(nlohmann::json& j, const RtpServerInfo& p)
	{

	}
}