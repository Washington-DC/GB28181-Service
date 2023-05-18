#pragma once
#include <crow_all.h>

class HttpServer
{
public:
	SINGLETON_DEFINE(HttpServer);
	void Start(int port);

private:
	HttpServer();
	crow::SimpleApp _app;
	crow::Blueprint _blueprint;

};
