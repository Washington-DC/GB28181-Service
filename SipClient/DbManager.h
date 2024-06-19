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
	std::vector<std::shared_ptr<dto::ZlmMP4Item>> Query(const std::string& stream_id, uint64_t start, uint64_t end);
	std::shared_ptr<dto::ZlmMP4Item> QueryOne(const std::string& stream_id, uint64_t start, uint64_t end);

private:
	bool _create_tables();
	bool _initialization();

	std::shared_ptr<sqlite3pp::database> _db = nullptr;
	DbManager() = default;
	bool _checked = false;
};

