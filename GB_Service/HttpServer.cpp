#include "pch.h"
#include "HttpServer.h"
#include "DeviceManager.h"

HttpServer::HttpServer()
	:_blueprint("v1")
{

	CROW_BP_ROUTE(_blueprint, "/")([]() {return "Hello World!"; });

	CROW_BP_ROUTE(_blueprint, "/devicelist")([]()
		{
			auto devices = DeviceManager::GetInstance()->GetDeviceList();
			auto doc = nlohmann::json::array();
			for (auto&& dev : devices)
			{
				doc.push_back(dev->toJson());
			}
			return nlohmann::json
			{
				{"status",0},
				{"data",doc}
			}.dump(4);
		}
	);

	CROW_BP_ROUTE(_blueprint, "/device/<string>/channellist")([](std::string device_id)
		{
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				auto channels = device->GetAllChannels();
				auto doc = nlohmann::json::array();
				for (auto&& channel : channels)
				{
					doc.push_back(channel->toJson());
				}

				return nlohmann::json
				{
					{"status",0},
					{"data",doc}
				}.dump(4);
			}
			else
			{
				return nlohmann::json
				{
					{"status",1},
					{"data","device not found"}
				}.dump(4);
			}
		}
	);


	CROW_BP_ROUTE(_blueprint, "/device/<string>")([](std::string device_id)
		{
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				return nlohmann::json
				{
					{"status",0},
					{"data",device->toJson()}
				}.dump(4);
			}
			else
			{
				return nlohmann::json
				{
					{"status",1},
					{"data","device not found"}
				}.dump(4);
			}
		}
	);

	CROW_BP_ROUTE(_blueprint, "/device/<string>/channel/<string>")([](std::string device_id, std::string channel_id)
		{
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				auto channel = device->GetChannel(channel_id);

				if (channel)
				{
					return nlohmann::json
					{
						{"status",0},
						{"data",channel->toJson()}
					}.dump(4);
				}
				else
				{
					return nlohmann::json
					{
						{"status",1},
						{"data","channel not found"}
					}.dump(4);
				}
			}
			else
			{
				return nlohmann::json
				{
					{"status",1},
					{"data","device not found"}
				}.dump(4);
			}
		}
	);


	CROW_BP_ROUTE(_blueprint, "/device/<string>/channel/<string>/play")([](std::string device_id, std::string channel_id)
		{
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				auto channel = device->GetChannel(channel_id);
				if (channel)
				{
					nlohmann::json doc = {
						{"ssrc",""},
						{"result","ok"}
					};

					return nlohmann::json
					{
						{"status",0},
						{"data",doc}
					}.dump(4);
				}
				else
				{
					return nlohmann::json
					{
						{"status",1},
						{"data","channel not found"}
					}.dump(4);
				}
			}
			else
			{
				return nlohmann::json
				{
					{"status",1},
					{"data","device not found"}
				}.dump(4);
			}
		}
	);

	_app.register_blueprint(_blueprint);
}


void HttpServer::Start(int port)
{
	_app.loglevel(crow::LogLevel::Critical).port(port).multithreaded().run_async();
}