#include "pch.h"
#include "DbManager.h"
#include "Utils.h"

bool DbManager::Init(const std::string& db_path)
{
	try
	{
		_db = std::make_shared<sqlite3pp::database>(ToUtf8String(db_path).c_str());
		return _initialization();
	}
	catch (const std::exception& e)
	{
		SPDLOG_ERROR(e.what());
		return false;
	}
}

bool DbManager::CreateTable(const std::string& name)
{
	auto sql = u8R"(
				CREATE TABLE IF NOT EXISTS "{}" (
					  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
					  "filepath" text,
					  "filename" text,
					  "starttime" integer,
					  "endtime" integer,
					  "duration" integer,
					  "filesize" integer
				)
				)";

	auto ret = _db->execute(fmt::format(sql, name).c_str());
	if (ret != SQLITE_OK)
	{
		SPDLOG_ERROR("create table \"{}\" failed", name);
		return false;
	}

	return true;
}

bool DbManager::AddFile(const std::string& name, const dto::ZlmMP4Item& item)
{
	auto sql = R"(INSERT INTO "{}" (
						"filepath",
						"filename", 
						"starttime", 
						"endtime", 
						"duration",
						"filesize"
						) 
						VALUES 
						(
							?1,?2,?3,?4,?5,?6
						))";

	auto text = fmt::format(sql, name);

	sqlite3pp::command cmd(*_db, text.c_str());
	cmd.bind(1, item.FilePath, sqlite3pp::copy);
	cmd.bind(2, item.FileName, sqlite3pp::copy);
	cmd.bind(3, (long long int)item.StartTime);
	cmd.bind(4, (long long int)(item.StartTime + item.TimeDuration));
	cmd.bind(5, (long long int)(item.TimeDuration));
	cmd.bind(6, (long long int)item.FileSize);

	auto ret = cmd.execute();
	return ret == SQLITE_OK;
}


std::vector<std::shared_ptr<dto::ZlmMP4Item>>
DbManager::Query(const std::string& stream_id, uint64_t start, uint64_t end) {
	std::vector<std::shared_ptr<dto::ZlmMP4Item>> files;
	try {
		auto text = R"(SELECT "filepath","filename","filesize","starttime","duration" FROM "{}" WHERE "endtime" >= {} AND "starttime" <= {} ORDER BY ID ASC)";
		auto sql = fmt::format(text, stream_id, start, end);
		if (_db)
		{
			sqlite3pp::query query(*_db.get(), sql.c_str());
			for (auto iter = query.begin(); iter != query.end(); ++iter)
			{
				auto video = std::make_shared<dto::ZlmMP4Item>();
				video->FilePath = ToMbcsString((*iter).get<const char*>(0));
				video->FileName = ToMbcsString((*iter).get<const char*>(1));
				video->FileSize = (*iter).get<long long int>(2);
				video->StartTime = (*iter).get<long long int>(3);
				video->TimeDuration = (*iter).get<long long int>(4);
				files.push_back(video);
			}
		}
	}
	catch (const std::exception& ex) {
		ErrorL << "Sqlite Query: " << ex.what();
	}
	return files;
}

std::shared_ptr<dto::ZlmMP4Item> DbManager::QueryOne(const std::string& stream_id, uint64_t start, uint64_t end)
{
	std::shared_ptr<dto::ZlmMP4Item> file = nullptr;
	try {
		auto text = R"(SELECT "filepath","filename","filesize","starttime","duration" FROM "{}" WHERE "starttime" = {} AND "endtime" = {} ORDER BY ID ASC)";
		auto sql = fmt::format(text, stream_id, start, end);
		if (_db)
		{
			sqlite3pp::query query(*_db.get(), sql.c_str());
			for (auto iter = query.begin(); iter != query.end(); ++iter)
			{
				file = std::make_shared<dto::ZlmMP4Item>();
				file->FilePath = ToMbcsString((*iter).get<const char*>(0));
				file->FileName = ToMbcsString((*iter).get<const char*>(1));
				file->FileSize = (*iter).get<long long int>(2);
				file->StartTime = (*iter).get<long long int>(3);
				file->TimeDuration = (*iter).get<long long int>(4);
				return file;
			}
		}
	}
	catch (const std::exception& ex) {
		ErrorL << "Sqlite Query: " << ex.what();
	}
	return file;
}


bool DbManager::_initialization()
{
	try
	{
		//1、关闭同步
		{
			auto sql = u8R"(PRAGMA synchronous = OFF)";
			auto ret = _db->execute(sql);
			if (ret != SQLITE_OK)
			{
				SPDLOG_ERROR("exec \"synchronous\" failed");
				return false;
			}
		}

		//2、启用WAL模式
		{
			auto sql = u8"PRAGMA journal_mode=WAL";
			sqlite3pp::query query(*_db.get(), sql);
			for (auto iter = query.begin(); iter != query.end(); ++iter)
			{
				auto state = ToMbcsString((*iter).get<const char*>(0));
				if (strstr(state.c_str(), "wal"))
				{
					SPDLOG_INFO("journal_mode : wal");
				}
				else
				{
					SPDLOG_ERROR("journal_mode : {}", state);
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		SPDLOG_ERROR(e.what());
		return false;
	}

	return true;
}
