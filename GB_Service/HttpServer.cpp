#include "pch.h"
#include "HttpServer.h"
#include "DeviceManager.h"
#include "StreamManager.h"

HttpServer::HttpServer()
	:_blueprint("v1")
{

	CROW_BP_ROUTE(_blueprint, "/")([]() {return "Hello World!"; });

	CROW_BP_ROUTE(_blueprint, "/devicelist")([this]()
		{
			auto devices = DeviceManager::GetInstance()->GetDeviceList();
			auto doc = nlohmann::json::array();
			for (auto&& dev : devices)
			{
				doc.push_back(dev->toJson());
			}
			return _mk_response(0, doc);
		}
	);

	CROW_BP_ROUTE(_blueprint, "/device/<string>/channellist")([this](std::string device_id)
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

				return _mk_response(0, doc);
			}
			else
			{
				return _mk_response(1, "device not found");
			}
		}
	);


	CROW_BP_ROUTE(_blueprint, "/device/<string>")([this](std::string device_id)
		{
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				return _mk_response(0, device->toJson());
			}
			else
			{
				return _mk_response(1, "device not found");
			}
		}
	);

	CROW_BP_ROUTE(_blueprint, "/device/<string>/channel/<string>")
		([this](std::string device_id, std::string channel_id)
		{
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "device not found");
			}

			auto channel = device->GetChannel(channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "channel not found");
			}

			return _mk_response(0, channel->toJson());
		}
	);


	CROW_BP_ROUTE(_blueprint, "/device/<string>/channel/<string>/play")
		([this](std::string device_id, std::string channel_id)
		{
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "device not found");
			}

			auto channel = device->GetChannel(channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "channel not found");
			}
			auto stream_id = fmt::format("{}_{}", device_id, channel_id);

			auto stream = StreamManager::GetInstance()->GetStream(stream_id);
			if (stream)
			{
				auto session = std::dynamic_pointer_cast<CallSession>(stream);
				if (session->IsConnected())
				{
					return _mk_response(400, "already exists");
				}
				else
				{

				}
			}
			return _mk_response(0, "ok");
		}
	);

	_app.register_blueprint(_blueprint);
}


void HttpServer::Start(int port)
{
	_app.loglevel(crow::LogLevel::Critical).port(port).multithreaded().run_async();
}