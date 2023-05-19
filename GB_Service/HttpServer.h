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
	std::string _mk_response(int status,Type t);

	crow::SimpleApp _app;
	crow::Blueprint _blueprint;

};


template<typename Type>
std::string HttpServer::_mk_response(int status, Type t)
{
	return nlohmann::json
	{
		{"status",status},
		{"data",t}
	}.dump(4);
}