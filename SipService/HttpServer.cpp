#include "pch.h"
#include "HttpServer.h"
#include "DeviceManager.h"
#include "StreamManager.h"
#include "SipRequest.h"
#include "SipServer.h"
#include "ZlmServer.h"
#include "DTO.h"


HttpServer::HttpServer()
	:_api_blueprint("v1")
	, _hook_blueprint("index/hook")
{
	CROW_ROUTE(_app, "/")([]() {return "Hello World!"; });

	CROW_BP_ROUTE(_api_blueprint, "/")([]() {return "Hello World !"; });

	CROW_BP_ROUTE(_api_blueprint, "/device/list")([this]()
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

	CROW_BP_ROUTE(_api_blueprint, "/device")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id");
			auto device_id = req.url_params.get("device_id");
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				return _mk_response(0, device->toJson());
			}
			return _mk_response(1, "", "device not found");
		}
	);

	CROW_BP_ROUTE(_api_blueprint, "/channel/list")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id");
			auto doc = nlohmann::json::array();
			auto device_id = req.url_params.get("device_id");
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				auto channels = device->GetAllChannels();
				for (auto&& channel : channels)
				{
					doc.push_back(channel->toJson());
				}

				return _mk_response(0, doc);
			}
			return _mk_response(1, "", "deviceid not exists");
		}
	);

	CROW_BP_ROUTE(_api_blueprint, "/channel")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");

			auto doc = nlohmann::json::array();
			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device)
			{
				auto channel = device->GetChannel(channel_id);
				if (channel == nullptr)
				{
					return _mk_response(1, "", "channel not found");
				}

				return _mk_response(0, channel->toJson());
			}
			else
			{
				return _mk_response(1, "", "device not found");
			}
		}
	);

	CROW_BP_ROUTE(_api_blueprint, "/play/start")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");
			return Play(device_id, channel_id);
		}
	);

	CROW_BP_ROUTE(_api_blueprint, "/play/stop")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");

			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "", "device not found");
			}

			auto channel = device->GetChannel(channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "", "channel not found");
			}
			auto stream_id = fmt::format("{}_{}", device_id, channel_id);
			auto stream = StreamManager::GetInstance()->GetStream(stream_id);
			if (stream)
			{
				auto session = std::dynamic_pointer_cast<CallSession>(stream);
				if (!session->IsConnected())
				{
					return _mk_response(400, "", "not play");
				}
				else
				{
					eXosip_call_terminate(SipServer::GetInstance()->GetSipContext(),
						session->GetCallID(), session->GetDialogID());
					session->SetConnected(false);

					return _mk_response(0, "", "ok");
				}
			}
			else
			{
				return _mk_response(400, "", "not play");
			}
		}
	);

	CROW_BP_ROUTE(_api_blueprint, "/play/stopall")
		([this]()
		{
			auto streams = StreamManager::GetInstance()->GetAllStream();
			for (auto&& stream : streams)
			{
				auto session = std::dynamic_pointer_cast<CallSession>(stream);
				if (session && session->IsConnected())
				{
					eXosip_call_terminate(SipServer::GetInstance()->GetSipContext(),
						session->GetCallID(), session->GetDialogID());
					session->SetConnected(false);
				}
			}
			return _mk_response(0, "", "ok");
		}
	);


	CROW_BP_ROUTE(_api_blueprint, "/stream/list")
		([this]()
		{
			auto streams = StreamManager::GetInstance()->GetAllStream();
			auto doc = nlohmann::json::array();
			for (auto&& s : streams)
			{
				doc.push_back(s->toJson());
			}
			return _mk_response(0, doc);
		}
	);


	CROW_BP_ROUTE(_api_blueprint, "/rtpserver/list")
		([this]()
		{
			return ZlmServer::GetInstance()->ListRtpServer();
		}
	);






	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------

	CROW_BP_ROUTE(_hook_blueprint, "/")([]() {return "Hello World !"; });


	//服务器定时上报，确认服务器是否在线
	CROW_BP_ROUTE(_hook_blueprint, "/on_server_keepalive").methods("POST"_method)([this](const crow::request& req)
		{
			ZlmServer::GetInstance()->UpdateHeartbeatTime();
			return _mk_response(0, "", "success");
		}
	);

	CROW_BP_ROUTE(_hook_blueprint, "/on_publish").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::ZlmStreamInfo>();
			LOG(INFO) << "Hook on_publish: " << info.Path() << "\tParams: " << info.Params;
			return nlohmann::json{
				{"code",0},
				{"msg",""},
				{"enable_rtsp",true},
				{"enable_rtmp",true},
				{"enable_hls",true}
			}.dump();
		}
	);

	//流注册或注销时触发此事件
	CROW_BP_ROUTE(_hook_blueprint, "/on_stream_changed").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::ZlmStreamInfo>();
			if (info.Regist)
			{
				if (info.OriginType == 3)
				{
					auto stream = StreamManager::GetInstance()->GetStream(info.Stream);
					if (stream)
					{
						auto session = std::dynamic_pointer_cast<CallSession>(stream);
						session->NotifyStreamReady();
					}
				}
			}
			else
			{
				StreamManager::GetInstance()->RemoveStream(info.Stream);
			}

			return _mk_response(0, "", "success");
		}
	);

	//流无人观看时事件，用户可以通过此事件选择是否关闭无人看的流
	CROW_BP_ROUTE(_hook_blueprint, "/on_stream_none_reader").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::ZlmStreamInfo>();
			LOG(INFO) << "on_stream_none_reader: " << info.Path();

			auto stream = StreamManager::GetInstance()->GetStream(info.Stream);
			if (stream)
			{
				auto session = std::dynamic_pointer_cast<CallSession>(stream);
				eXosip_call_terminate(SipServer::GetInstance()->GetSipContext(),
					session->GetCallID(), session->GetDialogID());
				session->SetConnected(false);
			}

			return nlohmann::json{
				{"close",true},
				{"code",0}
			}.dump(4);
		}
	);

	//流未找到事件，用户可以在此事件触发时，立即去拉流，这样可以实现按需拉流
	CROW_BP_ROUTE(_hook_blueprint, "/on_stream_not_found").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::ZlmStreamInfo>();
			LOG(INFO) << "on_stream_not_found: " << info.Path();

			if (info.App == "rtp")
			{
				auto pos = info.Stream.find_first_of('_');
				if (pos != std::string::npos)
				{
					auto device_id = info.Stream.substr(0, pos);
					auto channel_id = info.Stream.substr(pos + 1);
					auto ret = Play(device_id, channel_id);
					LOG(INFO) << "Play: " << ret;
				}
			}

			return _mk_response(0, "", "success");
		}
	);

	//调用openRtpServer 接口，rtp server 长时间未收到数据,执行此web hook
	CROW_BP_ROUTE(_hook_blueprint, "/on_rtp_server_timeout").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::RtpServerInfo>();
			LOG(INFO) << "on_rtp_server_timeout: " << info.StreamID;
			return _mk_response(0, "", "success");
		}
	);

	_app.register_blueprint(_api_blueprint);
	_app.register_blueprint(_hook_blueprint);
}

std::string HttpServer::Play(const std::string& device_id, const std::string& channel_id)
{
	auto device = DeviceManager::GetInstance()->GetDevice(device_id);
	if (device == nullptr)
	{
		return _mk_response(1, "", "device not found");
	}

	auto channel = device->GetChannel(channel_id);
	if (channel == nullptr)
	{
		return _mk_response(1, "", "channel not found");
	}
	auto stream_id = fmt::format("{}_{}", device_id, channel_id);

	InviteRequest::Ptr request = nullptr;
	auto stream = StreamManager::GetInstance()->GetStream(stream_id);
	if (stream)
	{
		auto session = std::dynamic_pointer_cast<CallSession>(stream);
		if (session->IsConnected())
		{
			return _mk_response(400, "", "already exists");
		}
	}

	request = std::make_shared<InviteRequest>(
		SipServer::GetInstance()->GetSipContext(), device, channel_id);

	request->SendCall();

	stream = StreamManager::GetInstance()->GetStream(stream_id);
	if (stream)
	{
		auto session = std::dynamic_pointer_cast<CallSession>(stream);
		auto ret = session->WaitForStreamReady();

		if (!ret)
		{
			return _mk_response(400, "", "timeout");
		}
	}

	return _mk_response(0, "", "ok");
}


void HttpServer::Start(int port)
{
	_app.loglevel(crow::LogLevel::Critical).port(port).multithreaded().run_async();
}