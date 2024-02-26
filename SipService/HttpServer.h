#pragma once
#include <crow_all.h>
#include "DeviceManager.h"

template<typename Req, typename First>
bool checkArgs(Req& req, const First& first) {
	return req.url_params.get(first);
}

template<typename Args, typename First, typename ...KeyTypes>
bool checkArgs(Args& args, const First& first, const KeyTypes &...keys) {
	return checkArgs(args, first) && checkArgs(args, keys...);
}

#define CHECK_ARGS(...)  \
    if(!checkArgs(req,##__VA_ARGS__)){ \
		return _mk_response(100,"","parameter not found: " #__VA_ARGS__);\
    }


class HttpServer
{
public:
	SINGLETON_DEFINE(HttpServer);
	std::future<void> Start(int port);

private:
	HttpServer();

	std::string Play(const std::string& device_id, const std::string& channel_id);
	std::string Playback(const std::string& device_id, const std::string& channel_id, int64_t start_time, int64_t end_time);

	template<typename Type>
	std::string _mk_response(int status, Type t, std::string msg = "ok");

	Device::Ptr GetDevice(const std::string& device_id);
	Channel::Ptr GetChannel(const std::string& device_id, const std::string& channel_id);

	crow::SimpleApp _app;
	crow::Blueprint _api_blueprint;
	crow::Blueprint _hook_blueprint;
};


template<typename Type>
std::string HttpServer::_mk_response(int status, Type t, std::string msg)
{
	return nlohmann::json
	{
		{"code",status},
		{"msg",msg},
		{"data",t}
	}.dump(4);
}