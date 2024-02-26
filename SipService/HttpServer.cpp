#include "pch.h"
#include "HttpServer.h"
#include "StreamManager.h"
#include "SipRequest.h"
#include "SipServer.h"
#include "ZlmServer.h"
#include "DTO.h"
#include "Utils.h"
#include "DbManager.h"

HttpServer::HttpServer()
	:_api_blueprint("v1")
	, _hook_blueprint("index/hook")
{
	CROW_ROUTE(_app, "/")([]() {return "Hello World!"; });

	CROW_BP_ROUTE(_api_blueprint, "/")([]() {return "Hello World !"; });

	//查询设备列表
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

	//查询某个设备信息
	CROW_BP_ROUTE(_api_blueprint, "/device")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id");
			auto device_id = req.url_params.get("device_id");
			auto device = GetDevice(device_id);
			if (device)
			{
				return _mk_response(0, device->toJson());
			}
			return _mk_response(1, "", "device not found");
		}
	);

	//查询某个设备的通道列表
	CROW_BP_ROUTE(_api_blueprint, "/channel/list")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id");
			auto doc = nlohmann::json::array();
			auto device_id = req.url_params.get("device_id");
			auto device = GetDevice(device_id);
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

	//查询某个设备的某个通道信息
	CROW_BP_ROUTE(_api_blueprint, "/channel")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");

			auto channel = GetChannel(device_id, channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "", "device or channel not found");
			}

			return _mk_response(0, channel->toJson());
		}
	);

	//实时流 播放
	CROW_BP_ROUTE(_api_blueprint, "/play/start")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");
			return Play(device_id, channel_id);
		}
	);

	//实时流 停止播放
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

			std::string stream_id = "";
			if (ZlmServer::GetInstance()->SinglePortMode())
			{
				stream_id = SSRC_Hex(channel->GetDefaultSSRC());
			}
			else
			{
				stream_id = fmt::format("{}_{}", device_id, channel_id);
			}

			if (stream_id.empty())
			{
				return _mk_response(400, "", "not play");
			}

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
					eXosip_call_terminate(session->exosip_context,
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


	//停止所有数据流播放
	CROW_BP_ROUTE(_api_blueprint, "/play/stopall")([this]()
		{
			auto streams = StreamManager::GetInstance()->GetAllStream();
			for (auto&& stream : streams)
			{
				auto session = std::dynamic_pointer_cast<CallSession>(stream);
				if (session && session->IsConnected())
				{
					eXosip_call_terminate(session->exosip_context,
						session->GetCallID(), session->GetDialogID());
					session->SetConnected(false);
				}
			}
			return _mk_response(0, "", "ok");
		}
	);

	//设置/调用/删除某个设备的预置位
	CROW_BP_ROUTE(_api_blueprint, "/preset")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id", "command", "preset");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");
			auto command = req.url_params.get("command");
			auto preset = std::stoi(req.url_params.get("preset"));

			auto cmd = -1;
			if (strcmp(command, "set") == 0) cmd = 0x81;
			else if (strcmp(command, "goto") == 0) cmd = 0x82;
			else if (strcmp(command, "del") == 0) cmd = 0x83;
			else return _mk_response(2, "", "command not suppport");

			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "", "device not found");
			}

			if (!device->IsRegistered())
			{
				return _mk_response(1, "", "device not online");
			}

			auto channel = device->GetChannel(channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "", "channel not found");
			}

			auto request = std::make_shared<PresetCtlRequest>(device->exosip_context, device, channel_id, cmd, 0, preset, 0);

			request->SendMessage();
			return _mk_response(0, "");
		}
	);

	//查询某个设备的预置位信息
	CROW_BP_ROUTE(_api_blueprint, "/preset/query")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");

			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "", "device not found");
			}
			if (!device->IsRegistered())
			{
				return _mk_response(1, "", "device not online");
			}
			auto channel = device->GetChannel(channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "", "channel not found");
			}

			//auto ctx = SipServer::GetInstance()->GetSipContext();
			auto request = std::make_shared<PresetRequest>(device->exosip_context, device, channel_id);
			request->SendMessage();

			request->SetWait();
			request->WaitResult();

			if (!request->IsFinished())
			{
				return _mk_response(400, "", "query timeout");
			}
			else
			{
				auto preset_list = request->GetPresetList();
				auto doc = nlohmann::json::array();
				for (auto&& p : preset_list)
				{
					doc.push_back({
						{"name",ToUtf8String(p.second)},
						{"id",p.first}
						});
				}

				return _mk_response(0, doc, "ok");
			}

			return _mk_response(0, "");
		}
	);

	//云台控制
	CROW_BP_ROUTE(_api_blueprint, "/ptz")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id", "command", "speed");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");
			auto command = req.url_params.get("command");
			auto speed = std::stoi(req.url_params.get("speed"));

			auto device = DeviceManager::GetInstance()->GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "", "device not found");
			}
			if (!device->IsRegistered())
			{
				return _mk_response(1, "", "device not online");
			}
			auto channel = device->GetChannel(channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "", "channel not found");
			}

			auto ctx = device->exosip_context;
			std::shared_ptr<MessageRequest> request = nullptr;
			if (strcmp(command, "left") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 1, 0, 0, speed, 0);
			else if (strcmp(command, "right") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 2, 0, 0, speed, 0);
			else if (strcmp(command, "up") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 0, 1, 0, speed, 0);
			else if (strcmp(command, "down") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 0, 2, 0, speed, 0);
			else if (strcmp(command, "upleft") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 1, 1, 0, speed, 0);
			else if (strcmp(command, "upright") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 2, 1, 0, speed, 0);
			else if (strcmp(command, "downleft") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 1, 2, 0, speed, 0);
			else if (strcmp(command, "downright") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 2, 2, 0, speed, 0);
			else if (strcmp(command, "zoomin") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 0, 0, 1, 0, 1);
			else if (strcmp(command, "zoomout") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 0, 0, 2, 0, 1);
			else if (strcmp(command, "stop") == 0)
				request = std::make_shared<PtzCtlRequest>(ctx, device, channel_id, 0, 0, 0, 0, 0);
			//else if (strcmp(command, "irisopen") == 0)
			//	request = std::make_shared<LensCtlRequest>(ctx, device, channel_id, 2, 0, speed, 0);
			//else if (strcmp(command, "irisclose") == 0)
			//	request = std::make_shared<LensCtlRequest>(ctx, device, channel_id, 1, 0, speed, 0);
			//else if (strcmp(command, "focusnear") == 0)
			//	request = std::make_shared<LensCtlRequest>(ctx, device, channel_id, 0, 1, 0, speed);
			//else if (strcmp(command, "focusfar") == 0)
			//	request = std::make_shared<LensCtlRequest>(ctx, device, channel_id, 0, 2, 0, speed);
			else return _mk_response(2, "", "command not suppport");

			request->SendMessage();
			return _mk_response(0, "");
		}
	);

	//查询所有流信息
	CROW_BP_ROUTE(_api_blueprint, "/stream/list")([this]()
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

	//查询rtpserver列表
	CROW_BP_ROUTE(_api_blueprint, "/rtpserver/list")([this]()
		{
			return ZlmServer::GetInstance()->ListRtpServer();
		}
	);

	//设置某个设备的收流IP
	CROW_BP_ROUTE(_api_blueprint, "/set/device/streamip")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "ip");
			auto device_id = req.url_params.get("device_id");
			auto ip = req.url_params.get("ip");

			auto device = GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "", "device not found");
			}

			device->SetStreamIP(ip);
			return _mk_response(0, "");
		}
	);

	//设置某个设备昵称
	CROW_BP_ROUTE(_api_blueprint, "/set/device/nickname")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "nickname");
			auto device_id = req.url_params.get("device_id");
			auto nickname = req.url_params.get("nickname");

			auto device = GetDevice(device_id);
			if (device == nullptr)
			{
				return _mk_response(1, "", "device not found");
			}

			device->SetNickName(nickname);
			DbManager::GetInstance()->AddOrUpdateDevice(device, true);
			return _mk_response(0, "");
		}
	);

	//设置某个设备通道昵称
	CROW_BP_ROUTE(_api_blueprint, "/set/channel/nickname")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id", "nickname");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");
			auto nickname = req.url_params.get("nickname");

			auto channel = GetChannel(device_id, channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "", "device or channel not found");
			}

			channel->SetNickName(nickname);
			DbManager::GetInstance()->AddOrUpdateChannel(device_id, channel, true);
			return _mk_response(0, "");
		}
	);

	//查询某个通道的SSRC
	CROW_BP_ROUTE(_api_blueprint, "/ssrc")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");

			auto stream_id = fmt::format("{}_{}", device_id, channel_id);
			auto stream = StreamManager::GetInstance()->GetStream(stream_id);

			if (stream)
			{
				auto session = std::dynamic_pointer_cast<CallSession>(stream);
				return _mk_response(0, nlohmann::json{ {"ssrc",session->GetSSRCInfo()->GetSSRC()} }, "ok");
			}
			else
			{
				return _mk_response(401, "", "stream not found");
			}
		}
	);

	//查询某个通道的默认流地址
	CROW_BP_ROUTE(_api_blueprint, "/defaultStreamID")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");

			auto channel = GetChannel(device_id, channel_id);
			if (channel == nullptr)
			{
				return _mk_response(1, "", "device or channel not found");
			}

			return _mk_response(0, nlohmann::json{ {"ssrc",channel->GetDefaultSSRC()} }, "ok");
		}
	);

	//录像文件查询
	CROW_BP_ROUTE(_api_blueprint, "/record/query")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id", "start_time", "end_time");
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

			auto start_time = std::stoi(req.url_params.get("start_time"));
			auto end_time = std::stoi(req.url_params.get("end_time"));

			auto request = std::make_shared<RecordRequest>(device->exosip_context, device, channel_id, start_time, end_time);
			request->SendMessage();

			request->SetWait();
			request->WaitResult();

			//等待时间完成之后，无论数据接收是否完成，都返回现有数据
			{
				auto items = request->GetRecordList();
				nlohmann::json doc = { {"count",items.size()} };
				doc["items"] = nlohmann::json::array();
				for (auto&& item : items)
				{
					doc["items"].push_back({
						{"filepath",item->FilePath},
						{"start_time",ISO8601ToTimeT(item->StartTime)},
						{"end_time",ISO8601ToTimeT(item->EndTime)},
						{"start_time_str",item->StartTime},
						{"end_time_str",item->EndTime}
						});
				}

				return _mk_response(0, doc, "ok");
			}
		}
	);

	//录像回放
	CROW_BP_ROUTE(_api_blueprint, "/record/play/start")([this](const crow::request& req)
		{
			CHECK_ARGS("device_id", "channel_id", "start_time", "end_time");
			auto device_id = req.url_params.get("device_id");
			auto channel_id = req.url_params.get("channel_id");
			auto start_time = req.url_params.get("start_time");
			auto end_time = req.url_params.get("end_time");
			return Playback(device_id, channel_id, std::stoi(start_time), std::stoi(end_time));
		}
	);

	//录像回放 停止, 根据回放的SSRC查找对应视频流，并关闭
	CROW_BP_ROUTE(_api_blueprint, "/record/play/stop")([this](const crow::request& req)
		{
			CHECK_ARGS("ssrc");
			auto ssrc = req.url_params.get("ssrc");

			std::string stream_id = SSRC_Hex(ssrc);
			if (stream_id.empty())
			{
				return _mk_response(400, "", "not play");
			}

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
					eXosip_call_terminate(session->exosip_context,
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



	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------

	CROW_BP_ROUTE(_hook_blueprint, "/")([]() {return "Hello World !"; });


	//服务器定时上报，确认服务器是否在线
	CROW_BP_ROUTE(_hook_blueprint, "/on_server_keepalive").methods("POST"_method)([this](const crow::request& req)
		{
			toolkit::EventPollerPool::Instance().getExecutor()->async([this]()
				{
					ZlmServer::GetInstance()->UpdateHeartbeatTime();
				});
			return _mk_response(0, "", "success");
		}
	);

	CROW_BP_ROUTE(_hook_blueprint, "/on_publish").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::ZlmStreamInfo>();
			SPDLOG_INFO("Hook on_publish: {}  \tParams:{}  ", info.Path(), info.Params);
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
	//TODO: 这里存在一个问题，在单端口模式下，如果这个流不存在，播放端使用device_id和channel_id去播放，和ssrc作为streamid冲突
	//可以考虑如下解决方案:
	// 1、在设备注册时，即生成每个channel对应的ssrc，播放前查询此ssrc作为stream_id去播放
	// 2、在ZLM端做映射，将device_id和channel_id映射到对应ssrc的数据
	CROW_BP_ROUTE(_hook_blueprint, "/on_stream_none_reader").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::ZlmStreamInfo>();
			SPDLOG_INFO("on_stream_none_reader: {}", info.Path());

			auto stream = StreamManager::GetInstance()->GetStream(info.Stream);
			if (stream)
			{
				auto session = std::dynamic_pointer_cast<CallSession>(stream);
				eXosip_call_terminate(session->exosip_context,
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
			SPDLOG_INFO("on_stream_not_found: {}", info.Path());

			if (info.App == "rtp")
			{
				auto task = toolkit::EventPollerPool::Instance().getExecutor()->async([this, info]()
					{
						//发送INVITE请求
						auto pos = info.Stream.find_first_of('_');
						if (pos != std::string::npos)
						{
							auto device_id = info.Stream.substr(0, pos);
							auto channel_id = info.Stream.substr(pos + 1);
							auto ret = Play(device_id, channel_id);
							SPDLOG_INFO("Play: {}", ret);
						}
						else
						{
							auto&& devices = DeviceManager::GetInstance()->GetDeviceList();
							for (auto&& device : devices)
							{
								auto&& channels = device->GetAllChannels();
								for (auto&& channel : channels)
								{
									if (channel->GetDefaultStreamID() == info.Stream)
									{
										auto ret = Play(device->GetDeviceID(), channel->GetChannelID());
										SPDLOG_INFO("Play: ", ret);
									}
								}
							}
						}
					});

				return _mk_response(0, "", "success");
			}
			return _mk_response(2, "", "stream app not supported");
		}
	);

	//调用openRtpServer 接口，rtp server 长时间未收到数据,执行此web hook
	CROW_BP_ROUTE(_hook_blueprint, "/on_rtp_server_timeout").methods("POST"_method)([this](const crow::request& req)
		{
			auto info = nlohmann::json::parse(req.body).get<dto::RtpServerInfo>();
			SPDLOG_INFO("on_rtp_server_timeout: {}", info.StreamID);
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

	if (!device->IsRegistered())
	{
		return _mk_response(1, "", "device not online");
	}

	auto channel = device->GetChannel(channel_id);
	if (channel == nullptr)
	{
		return _mk_response(1, "", "channel not found");
	}

	InviteRequest::Ptr request = nullptr;

	std::string stream_id = "";
	if (ZlmServer::GetInstance()->SinglePortMode())
	{
		stream_id = SSRC_Hex(channel->GetDefaultSSRC());
	}
	else
	{
		stream_id = fmt::format("{}_{}", device_id, channel_id);
	}

	if (!stream_id.empty())
	{
		auto stream = StreamManager::GetInstance()->GetStream(stream_id);
		if (stream)
		{
			auto session = std::dynamic_pointer_cast<CallSession>(stream);
			if (session->IsConnected())
			{
				return _mk_response(400, "", "already exists");
			}
		}
	}

	request = std::make_shared<InviteRequest>(device->exosip_context, device, channel_id, channel->GetDefaultSSRC(), stream_id);
	request->SendCall();

	auto stream = StreamManager::GetInstance()->GetStream(stream_id);
	if (stream)
	{
		auto session = std::dynamic_pointer_cast<CallSession>(stream);
		auto ret = session->WaitForStreamReady(ZlmServer::GetInstance()->MaxPlayWaitTime());

		if (!ret)
		{
			return _mk_response(400, "", "timeout");
		}

		return _mk_response(0, nlohmann::json{ {"ssrc",session->GetSSRCInfo()->GetSSRC()} }, "ok");
	}

	return _mk_response(401, "", "unknow error");
}


//录像回放
std::string HttpServer::Playback(const std::string& device_id, const std::string& channel_id, int64_t start_time, int64_t end_time)
{
	auto device = DeviceManager::GetInstance()->GetDevice(device_id);
	if (device == nullptr)
	{
		return _mk_response(1, "", "device not found");
	}

	if (!device->IsRegistered())
	{
		return _mk_response(1, "", "device not online");
	}

	auto channel = device->GetChannel(channel_id);
	if (channel == nullptr)
	{
		return _mk_response(1, "", "channel not found");
	}

	InviteRequest::Ptr request = nullptr;

	//生成ssrc以及stream_id
	auto ssrc = SSRCConfig::GetInstance()->GenerateSSRC(SSRCConfig::Mode::Playback);
	std::string stream_id = SSRC_Hex(ssrc);

	if (!stream_id.empty())
	{
		//判断该ssrc/streamid对应的视频流是否已存在
		auto stream = StreamManager::GetInstance()->GetStream(stream_id);
		if (stream)
		{
			auto session = std::dynamic_pointer_cast<CallSession>(stream);
			if (session->IsConnected())
			{
				return _mk_response(400, "", "already exists");
			}
		}
	}

	//发送invite请求
	request = std::make_shared<InviteRequest>(device->exosip_context, device, channel_id, ssrc, stream_id,
		SSRCConfig::Mode::Playback, start_time, end_time);
	request->SendCall();

	auto stream = StreamManager::GetInstance()->GetStream(stream_id);
	if (stream)
	{
		auto session = std::dynamic_pointer_cast<CallSession>(stream);
		//等待流注册或一段时间后，
		auto ret = session->WaitForStreamReady(ZlmServer::GetInstance()->MaxPlayWaitTime());
		//超时
		if (!ret)
			return _mk_response(400, "", "timeout");

		//视频流已注册
		return _mk_response(0, nlohmann::json{ {"ssrc",session->GetSSRCInfo()->GetSSRC()} }, "ok");
	}

	return _mk_response(401, "", "unknow error");
}

std::shared_ptr<Device> HttpServer::GetDevice(const std::string& device_id)
{
	return DeviceManager::GetInstance()->GetDevice(device_id);
}

std::shared_ptr<Channel> HttpServer::GetChannel(const std::string& device_id, const std::string& channel_id)
{
	auto device = DeviceManager::GetInstance()->GetDevice(device_id);
	if (device)
	{
		return device->GetChannel(channel_id);
	}
	return nullptr;
}


std::future<void> HttpServer::Start(int port)
{
	return _app.loglevel(crow::LogLevel::Critical).port(port).multithreaded().run_async();
}