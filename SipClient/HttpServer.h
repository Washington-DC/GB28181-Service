#pragma once
#include <crow_all.h>

namespace dto
{
	struct ZlmStreamInfo
	{
		std::string MediaServerID;
		std::string App;
		std::string Stream;
		std::string Schema;
		std::string Vhost;
		int	OriginType;
		bool Regist;
		uint16_t Port;
		std::string IP;
		std::string Params;
		std::string Path() { return App + "/" + Stream; }
	};

	void from_json(const nlohmann::json& j, ZlmStreamInfo& info);
	void to_json(nlohmann::json& j, const ZlmStreamInfo& p);
}

class HttpServer
{
	using OnStreamChangedCallback = std::function<void(const std::string& app, const std::string& stream, bool regist)>;
public:
    SINGLETON_DEFINE(HttpServer);
	std::future<void> Start(int port);
	void AddStreamChangedCallback(OnStreamChangedCallback func);

private:
    HttpServer();
	std::vector<OnStreamChangedCallback> _vec_stream_changed_callback;
	std::mutex _mtx;
    crow::SimpleApp _app;
    crow::Blueprint _hook_blueprint;
};