#include "pch.h"
#include "DbManager.h"
#include "Utils.h"

bool DbManager::Init(const std::string& db_path)
{
	try
	{
		_db = std::make_shared<sqlite3pp::database>(ToUtf8String(db_path).c_str());
		return _create_tables();
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << e.what();
		return false;
	}
}

bool DbManager::AddOrUpdateDevice(Device::Ptr device, bool update)
{
	if (!_checked)
		return false;

	std::string text = "";
	if (!update)
	{
		text = R"(INSERT INTO "Device" (
						"device_id",
						"name", 
						"nickname", 
						"ip", 
						"port", 
						"transport", 
						"manufacturer", 
						"model", 
						"stream_ip", 
						"channel_count", 
						"parent_id", 
						"regist_time", 
						"last_time",
						"update_time"
						) 
						VALUES 
						(
							?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,datetime('now','localtime')
						))";
	}
	else
	{
		text = R"(UPDATE "Device" 
						SET "name" = ?2,
						"nickname" = ?3,
						"ip" = ?4,
						"port" = ?5,
						"transport" = ?6,
						"manufacturer" = ?7,
						"model" = ?8,
						"stream_ip" = ?9,
						"channel_count" = ?10,
						"parent_id" = ?11,
						"regist_time" = ?12,
						"last_time" = ?13,
						"update_time" = datetime('now','localtime')
						WHERE
							"device_id" = ?1)";
	}

	sqlite3pp::command cmd(*_db, text.c_str());
	cmd.bind(1, device->GetDeviceID(), sqlite3pp::copy);
	cmd.bind(2, ToUtf8String(device->GetName()), sqlite3pp::copy);
	cmd.bind(3, ToUtf8String(device->GetNickName()), sqlite3pp::copy);
	cmd.bind(4, device->GetIP(), sqlite3pp::copy);
	cmd.bind(5, device->GetPort(), sqlite3pp::copy);
	cmd.bind(6, device->GetTransport(), sqlite3pp::copy);
	cmd.bind(7, device->GetManufacturer(), sqlite3pp::copy);
	cmd.bind(8, device->GetModel(), sqlite3pp::copy);
	cmd.bind(9, device->GetStreamIP(), sqlite3pp::copy);
	cmd.bind(10, device->GetChannelCount());
	cmd.bind(11, device->GetParentID(), sqlite3pp::copy);
	cmd.bind(12, LocalTime(device->GetRegistTime()), sqlite3pp::copy);
	cmd.bind(13, LocalTime(device->GetLastTime()), sqlite3pp::copy);

	auto ret = cmd.execute();
	return ret == SQLITE_OK;
}

bool DbManager::AddOrUpdateChannel(const std::string& device_id, Channel::Ptr channel, bool update)
{
	if (!_checked)
		return false;

	std::string text = "";
	if (!update)
	{
		text = R"(INSERT INTO "Channel" (
										"channel_id",
										"device_id",
										"name",
										"nickname",
										"ip",
										"manufacturer",
										"model",
										"ssrc",
										"owner",
										"civil_code",
										"address",
										"parental",
										"secrety",
										"stream_num",
										"ptz_type",
										"download_speed"
									)
									VALUES
										(
											?1,
											?2,
											?3,
											?4,
											?5,
											?6,
											?7,
											?8,
											?9,
											?10,
											?11,
											?12,
											?13,
											?14,
											?15,
											?16
										))";
	}
	else
	{
		text = R"(UPDATE "Channel" 
					SET 
						"name" = ?3,
						"nickname" = ?4,
						"ip" = ?5,
						"manufacturer" = ?6,
						"model" = ?7,
						"ssrc" = ?8,
						"owner" = ?9,
						"civil_code" = ?10,
						"address" = ?11,
						"parental" = ?12,
						"secrety" = ?13,
						"stream_num" = ?14,
						"ptz_type" = ?15,
						"download_speed" = ?16
					WHERE
						"channel_id" = ?1
					AND
						"device_id" = ?2)";
	}

	try
	{
		sqlite3pp::command cmd(*_db.get(), text.c_str());
		cmd.bind(1, channel->GetChannelID(), sqlite3pp::copy);
		cmd.bind(2, device_id, sqlite3pp::copy);
		cmd.bind(3, ToUtf8String(channel->GetName()), sqlite3pp::copy);
		cmd.bind(4, ToUtf8String(channel->GetNickName()), sqlite3pp::copy);
		cmd.bind(5, channel->GetIpAddress(), sqlite3pp::copy);
		cmd.bind(6, channel->GetManufacturer(), sqlite3pp::copy);
		cmd.bind(7, channel->GetModel(), sqlite3pp::copy);
		cmd.bind(8, channel->GetDefaultSSRC(), sqlite3pp::copy);
		cmd.bind(9, channel->GetOwner(), sqlite3pp::copy);
		cmd.bind(10, channel->GetCivilCode(), sqlite3pp::copy);
		cmd.bind(11, channel->GetAddress(), sqlite3pp::copy);
		cmd.bind(12, channel->GetParental(), sqlite3pp::copy);
		cmd.bind(13, channel->GetSecrety(), sqlite3pp::copy);
		cmd.bind(14, channel->GetStreamNum(), sqlite3pp::copy);
		cmd.bind(15, channel->GetPtzType(), sqlite3pp::copy);
		cmd.bind(16, channel->GetDownloadSpeed(), sqlite3pp::copy);

		auto ret = cmd.execute();
		return ret == SQLITE_OK;
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << e.what();
		return false;
	}
}

std::vector<Device::Ptr> DbManager::GetDeviceList()
{
	std::vector<Device::Ptr> devices;
	auto sql = u8R"(SELECT
						"device_id",
						"name",
						"nickname",
						"ip",
						"port",
						"transport",
						"manufacturer",
						"model",
						"stream_ip",
						"channel_count",
						"parent_id",
						strftime('%s', "regist_time"),
						strftime('%s', "last_time")
					FROM
						Device)";

	sqlite3pp::query qry(*_db, sql);
	for (auto iter : qry)
	{
		auto device = std::make_shared<Device>();
		device->SetDeviceID(iter.get<char const*>(0));
		device->SetName(ToMbcsString(iter.get<char const*>(1)));
		device->SetNickName(ToMbcsString(iter.get<char const*>(2)));
		device->SetIP(iter.get<char const*>(3));
		device->SetPort(iter.get<char const*>(4));
		device->SetTransport(iter.get<char const*>(5));
		device->SetManufacturer(iter.get<char const*>(6));
		device->SetModel(iter.get<char const*>(7));
		device->SetStreamIP(iter.get<char const*>(8));
		device->SetChannelCount(/*iter.get<int>(9)*/0);
		device->SetParentID(iter.get<char const*>(10));
		device->UpdateRegistTime(iter.get<long long>(11));
		device->UpdateLastTime(iter.get<long long>(12));

		devices.push_back(device);
	}
	return devices;
}

std::vector<Channel::Ptr> DbManager::GetChannelList(const std::string& device_id)
{
	std::vector<Channel::Ptr> channels;
	auto text = u8R"(SELECT
						"channel_id",
						"name",
						"nickname",
						"ip",
						"manufacturer",
						"model",
						"ssrc",
						"owner",
						"civil_code",
						"address",
						"parental",
						"secrety",
						"stream_num",
						"ptz_type",
						"download_speed"
					FROM
						Channel 
					WHERE
						device_id = '{}')";
	auto sql = fmt::format(text, device_id);
	sqlite3pp::query qry(*_db, sql.c_str());
	for (auto iter : qry)
	{
		auto channel = std::make_shared<Channel>();
		channel->SetChannelID(iter.get<char const*>(0));
		channel->SetName(ToMbcsString(iter.get<char const*>(1)));
		channel->SetNickName(ToMbcsString(iter.get<char const*>(2)));
		channel->SetIpAddress(iter.get<char const*>(3));
		channel->SetManufacturer(iter.get<char const*>(4));
		channel->SetModel(iter.get<char const*>(5));
		channel->SetDefaultSSRC(iter.get<char const*>(6));
		channel->SetOwner(iter.get<char const*>(7));
		channel->SetCivilCode(iter.get<char const*>(8));
		channel->SetAddress(iter.get<char const*>(9));
		channel->SetParental(iter.get<char const*>(10));
		channel->SetSecrety(iter.get<char const*>(11));
		channel->SetStreamNum(iter.get<char const*>(12));
		channel->SetPtzType(iter.get<char const*>(13));
		channel->SetDownloadSpeed(iter.get<char const*>(14));

		channels.push_back(channel);
	}
	return channels;
}

bool DbManager::_create_tables()
{
	auto sql = u8R"(PRAGMA synchronous = OFF)";
	auto ret = _db->execute(sql);
	if (ret != SQLITE_OK)
	{
		LOG(ERROR) << "exec \"synchronous\" failed";
		return false;
	}

	sql = u8R"(
					CREATE TABLE IF NOT EXISTS "Device" (
						  "device_id" text NOT NULL PRIMARY KEY,
						  "name" text,
						  "nickname" text,
						  "ip" text,
						  "port" integer,
						  "transport" text,
						  "manufacturer" text,
						  "model" text,
						  "stream_ip" text,
						  "channel_count" integer,
						  "parent_id" text,
						  "regist_time" datetime,
						  "last_time" datetime,
						  "created_time" datetime not null default (datetime('now','localtime')),
						  "update_time" datetime not null default (datetime('now','localtime'))
					)
					)";

	ret = _db->execute(sql);
	if (ret != SQLITE_OK)
	{
		LOG(ERROR) << "create table \"device\" failed";
		return false;
	}

	sql = u8R"(
					CREATE TABLE IF NOT EXISTS "Channel" (
						  "channel_id" text NOT NULL PRIMARY KEY,
						  "device_id" text,
						  "name" text,
						  "nickname" text,
						  "ip" text,
						  "manufacturer" text,
						  "model" text,
						  "ssrc" text,
						  "owner" text,
						  "civil_code" text,
						  "address" text,
						  "parental" text,
						  "secrety" text,
						  "stream_num" text,
						  "ptz_type" text,
						  "download_speed" text,
						  "created_time" datetime not null default (datetime('now','localtime')),
						  "update_time" datetime not null default (datetime('now','localtime'))
					)
					)";

	ret = _db->execute(sql);
	if (ret != SQLITE_OK)
	{
		LOG(ERROR) << "create table \"channel\" failed";
		return false;
	}

	sql = u8R"(
					CREATE TABLE IF NOT EXISTS "StreamLog" (
						  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
						  "device_id" text,
						  "channel_id" text,
						  "ssrc" text,
						  "stream_id" text,
						  "operate" text,
						  "time" datetime not null default (datetime('now','localtime'))
					)
					)";

	ret = _db->execute(sql);
	if (ret != SQLITE_OK)
	{
		LOG(ERROR) << "create table \"stream_log\" failed";
		return false;
	}

	sql = u8R"(
					CREATE TABLE IF NOT EXISTS "Log" (
						  "id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
						  "info" text,
						  "time" datetime not null default (datetime('now','localtime'))
					)
					)";

	ret = _db->execute(sql);
	if (ret != SQLITE_OK)
	{
		LOG(ERROR) << "create table \"log\" failed";
		return false;
	}

	//sql = u8R"(
	//				CREATE TRIGGER IF NOT EXISTS device_update_time AFTER UPDATE   
	//					ON Device  
	//					BEGIN  
	//					UPDATE Device SET update_time = datetime('now','localtime') WHERE device_id = old.device_id; 
	//					END
	//				)";

	//ret = _db->execute(sql);
	//if (ret != SQLITE_OK)
	//{
	//	LOG(ERROR) << "create trigger \"device_update_time\" failed";
	//	return false;
	//}

	sql = u8R"(
					CREATE TRIGGER IF NOT EXISTS channel_update_time AFTER UPDATE   
						ON Channel  
						BEGIN  
						UPDATE Channel SET update_time = datetime('now','localtime') WHERE channel_id = old.channel_id AND device_id = old.device_id; 
						END
					)";

	ret = _db->execute(sql);
	if (ret != SQLITE_OK)
	{
		LOG(ERROR) << "create trigger \"channel_update_time\" failed";
		return false;
	}
	_checked = true;
	return true;
}
