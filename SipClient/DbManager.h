#pragma once
#include <sqlite3pp.h>
#include "Device.h"
#include "HttpServer.h"

class DbManager
{
public:
	SINGLETON_DEFINE(DbManager);

	bool Init(const std::string& db_path);
	bool CreateTable(const std::string& name);
	bool AddFile(const std::string& name, const dto::ZlmMP4Item& item);

private:
	bool _create_tables();
	bool _initialization();

	std::shared_ptr<sqlite3pp::database> _db = nullptr;
	DbManager() = default;
	bool _checked = false;
};

