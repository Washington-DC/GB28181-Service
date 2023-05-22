#pragma once
#include <crow_all.h>

class HttpServer
{
public:
	SINGLETON_DEFINE(HttpServer);
	void Start(int port);

private:
	HttpServer();

	template<typename Type>
	std::string _mk_response(int status, Type t, std::string msg = "ok");

	crow::SimpleApp _app;
	crow::Blueprint _api_blueprint;
	crow::Blueprint _hook_blueprint;
};


template<typename Type>
std::string HttpServer::_mk_response(int status, Type t, std::string msg)
{
	return nlohmann::json
	{
		{"status",status},
		{"message",msg},
		{"data",t}
	}.dump(4);
}