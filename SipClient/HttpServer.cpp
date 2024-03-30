#include "pch.h"
#include "HttpServer.h"

namespace dto
{
	/// @brief 将json信息反序列化到实际对象
	/// @param j json内容
	/// @param info 输出的实例
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

	//用不到，但是要有这个接口
	void to_json(nlohmann::json& j, const ZlmStreamInfo& p)
	{

	}
}


HttpServer::HttpServer()
	:_hook_blueprint("index/hook")
{
	//流注册或注销时触发此事件
	CROW_BP_ROUTE(_hook_blueprint, "/on_stream_changed").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::ZlmStreamInfo>();
			std::scoped_lock<std::mutex> g(_mtx);
			if (!_vec_stream_changed_callback.empty())
			{
				for (auto&& func : _vec_stream_changed_callback) {
					func(info.App, info.Stream, info.Regist);
				}
			}
			return crow::json::wvalue({ {"code", 0}, {"msg", "success"} });
		}
	);

	_app.register_blueprint(_hook_blueprint);
}


std::future<void> HttpServer::Start(int port)
{
	return _app.loglevel(crow::LogLevel::Critical).port(port).multithreaded().run_async();
}


void HttpServer::AddStreamChangedCallback(OnStreamChangedCallback func)
{
	std::scoped_lock<std::mutex> g(_mtx);
	_vec_stream_changed_callback.push_back(func);
}
