#pragma once

namespace dto
{
	struct StreamChangedInfo
	{
		std::string MediaServerID;
		std::string App;
		std::string Stream;
		std::string Schema;
		std::string Vhost;
		int	OriginType;
		bool Regist;

		std::string Path() { return App + "/" + Stream; }
	};


	struct RtpServerInfo
	{
		int LocalPort;
		bool ReusePort;
		uint32_t SSRC;
		std::string StreamID;
		int TcpMode;
		std::string MediaServerID;
	};

	struct ResponseInfo
	{
		int Code;
		int Port;
	};


	void from_json(const nlohmann::json& j, StreamChangedInfo& info);
	void to_json(nlohmann::json& j, const StreamChangedInfo& p);
	void from_json(const nlohmann::json& j, ResponseInfo& info);
	void to_json(nlohmann::json& j, const ResponseInfo& p);
	void from_json(const nlohmann::json& j, RtpServerInfo& info);
	void to_json(nlohmann::json& j, const RtpServerInfo& p);

}
