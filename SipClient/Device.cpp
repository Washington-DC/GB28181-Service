#include "pch.h"
#include "Device.h"
#include "HttpClient.h"
#include "HttpServer.h"
#include "FileTime.h"
#include "NetUtils.h"
//起始端口号
static int start_port = 20000;
//最大的SN
static int SN_MAX = 99999999;
//起始SN
static int sn = 0;

static int get_port()
{
	start_port++;
	if (start_port > 65500)
	{
		start_port = 20000;
	}
	return start_port;
}

static int get_sn()
{
	if (sn >= SN_MAX)
	{
		sn = 0;
	}
	sn++;
	return sn;
}

#define CALLBACK_TEMPLATE(F) (std::bind(&SipDevice::F, this, std::placeholders::_1))


SipDevice::SipDevice(const std::shared_ptr<DeviceInfo> info, std::shared_ptr<SipServerInfo> sip_server_info)
{
	this->ID = info->ID;
	this->IP = info->IP;
	this->Port = info->Port;
	this->Protocol = info->Protocol;
	this->Name = info->Name;
	this->Manufacturer = info->Manufacturer;
	this->HeartbeatInterval = info->HeartbeatInterval;
	this->Channels = info->Channels;

	this->_sip_server_info = sip_server_info;
}


bool SipDevice::Init()
{
	_sip_context = eXosip_malloc();
	if (OSIP_SUCCESS != eXosip_init(_sip_context))
	{
		SPDLOG_ERROR("eXosip_init failed");
		return false;
	}

	//部分服务器不会根据设备信息来判断厂家，而是根据UA来判断
	eXosip_set_option(_sip_context, EXOSIP_OPT_SET_HEADER_USER_AGENT, this->Manufacturer.c_str());

	_from_uri = fmt::format("sip:{}@{}:{}", this->ID, this->IP, this->Port);
	_contact_url = fmt::format("sip:{}@{}:{}", this->ID, this->IP, this->Port);
	_proxy_uri = fmt::format("sip:{}@{}:{}", _sip_server_info->ID, _sip_server_info->IP, _sip_server_info->Port);

	HttpServer::GetInstance()->AddStreamChangedCallback(std::bind(&SipDevice::OnStreamChanedCallback, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	return true;
}


bool SipDevice::StartSipClient()
{
	//创建socket，Port如果是0，则会随机使用端口
	if (OSIP_SUCCESS != eXosip_listen_addr(_sip_context, this->Protocol, NULL, this->Port, AF_INET, 0))
	{
		SPDLOG_ERROR("eXosip_listen_addr");
		eXosip_quit(_sip_context);
		return false;
	}

	_is_running = true;
	_sip_thread = std::make_shared<std::thread>(&SipDevice::SipRecvEventThread, this);
	//清理注册信息
	eXosip_clear_authentication_info(_sip_context);
	//构建注册信息
	osip_message_t* register_msg = nullptr;
	_register_id = eXosip_register_build_initial_register(
		_sip_context, _from_uri.c_str(), _proxy_uri.c_str(), _contact_url.c_str(), 3600, &register_msg);
	if (register_msg == nullptr)
	{
		SPDLOG_ERROR("eXosip_register_build_initial_register failed");
	}
	//发送注册信息
	eXosip_lock(_sip_context);
	auto ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_unlock(_sip_context);

	if (ret == OSIP_SUCCESS)
	{
		SPDLOG_INFO("发送注册信息");
		return true;
	}
	else
	{
		SPDLOG_ERROR("发送注册信息失败");
		return false;
	}
}

void SipDevice::StopSipClient()
{
	_is_running = false;

	// 退出时，停止推送所有数据
	std::scoped_lock<std::mutex> g(_session_mutex);
	for (auto&& [_, session] : _session_map)
	{
		session->Stop();
	}

	_register_success = false;
	_is_heartbeat_running = false;

	if (_subscription_thread)
	{
		_subscription_condition.notify_one();
		_subscription_thread->join();
		_subscription_thread = nullptr;
	}

	if (_heartbeat_thread)
	{
		_heartbeat_condition.notify_one();
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}

	if (_sip_thread)
	{
		_sip_thread->join();
		_sip_thread = nullptr;
	}

	if (_sip_context)
	{
		eXosip_quit(_sip_context);
		_sip_context = nullptr;
	}
}


bool SipDevice::Logout()
{
	_is_heartbeat_running = false;

	if (_subscription_thread)
	{
		_subscription_condition.notify_one();
		_subscription_thread->join();
		_subscription_thread = nullptr;
	}

	if (_heartbeat_thread)
	{
		_heartbeat_condition.notify_one();
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}

	SPDLOG_INFO("发送注销命令");
	osip_message_t* register_msg = nullptr;
	auto ret = eXosip_register_build_register(_sip_context, _register_id, 0, &register_msg);
	if (ret != OSIP_SUCCESS)
	{
		SPDLOG_ERROR("eXosip_register_build_initial_register failed");
		return false;
	}

	osip_contact_t* contact = nullptr;
	osip_message_get_contact(register_msg, 0, &contact);

	//注销时，设置expires为0即可
	auto info = fmt::format("{};expires=0", _contact_url);
	osip_list_remove(&register_msg->contacts, 0);
	osip_message_set_contact(register_msg, info.c_str());
	osip_message_set_header(register_msg, "Logout-Reason", "logout");

	eXosip_lock(_sip_context);
	ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	//注销后，清理注册内容
	eXosip_clear_authentication_info(_sip_context);
	eXosip_unlock(_sip_context);

	return true;
}

/// @brief 轮询接收数据
void SipDevice::SipRecvEventThread()
{
	_event_processor_map = {

		// 查询命令（目录查询、录像文件查询、设备参数等等）
		{EXOSIP_MESSAGE_NEW,			CALLBACK_TEMPLATE(OnMessageNew)},
		// 注册成功
		{EXOSIP_REGISTRATION_SUCCESS,	CALLBACK_TEMPLATE(OnRegistrationSuccess)},
		// 注册失败
		{EXOSIP_REGISTRATION_FAILURE,	CALLBACK_TEMPLATE(OnRegistrationFailed)},
		// 服务端删除此设备后，发送请求会收到此回复
		{EXOSIP_MESSAGE_REQUESTFAILURE, CALLBACK_TEMPLATE(OnMessageRequestFailed)},
		// 消息订阅( 如：移动设备位置信息订阅 )
		{EXOSIP_IN_SUBSCRIPTION_NEW,	CALLBACK_TEMPLATE(OnInSubscriptionNew)},


		// 请求会话
		{EXOSIP_CALL_INVITE,			CALLBACK_TEMPLATE(OnCallInvite)},
		// 确认请求，开始推流
		{EXOSIP_CALL_ACK,				CALLBACK_TEMPLATE(OnCallACK)},
		// 关闭推流
		{EXOSIP_CALL_CLOSED,			CALLBACK_TEMPLATE(OnCallClosed)},
		// 播放命令(暂停、继续播放)
		{EXOSIP_CALL_MESSAGE_NEW,		CALLBACK_TEMPLATE(OnCallMessageNew)}
	};

	eXosip_event_t* event = nullptr;
	while (_is_running)
	{
		event = eXosip_event_wait(_sip_context, 0, 50);
		if (event)
		{
			SPDLOG_INFO("------------- Receive: {}", magic_enum::enum_name<eXosip_event_type>(event->type));
		}

		//对于某些内容，eXosip可以自动执行
		{
			eXosip_lock(_sip_context);
			eXosip_automatic_action(_sip_context);
			eXosip_unlock(_sip_context);
		}

		if (event == nullptr)
			continue;

		auto proc = _event_processor_map.find(event->type);
		if (proc != _event_processor_map.end())
		{
			_event_processor_map[event->type](event);
		}

		toolkit::EventPollerPool::Instance().getExecutor()->async([this,event]()
		{
			auto proc = _event_processor_map.find(event->type);
			if (proc != _event_processor_map.end())
			{
				_event_processor_map[event->type](event);
			}

			eXosip_event_free(event);
		});
	}
}


void SipDevice::OnRegistrationFailed(eXosip_event_t* event)
{
	_register_success = false;
	_is_heartbeat_running = false;
	if (event->response == nullptr)
	{
		SPDLOG_ERROR("注册失败");
		return;
	}
	SPDLOG_ERROR("注册失败: {}", event->response->status_code);
	_is_heartbeat_running = false;
	if (_heartbeat_thread)
	{
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}
	//当服务端回复401或403时，表示已经发送了一次消息
	if (event->response->status_code == 401 || event->response->status_code == 403)
	{
		osip_www_authenticate_t* www_authenticate_header = nullptr;
		osip_message_get_www_authenticate(event->response, 0, &www_authenticate_header);
		if (www_authenticate_header)
		{
			//获取服务器域
			_sip_server_domain = osip_strdup_without_quote(www_authenticate_header->realm);
			//使用md5摘要生成注册内容
			eXosip_add_authentication_info(
				_sip_context, this->ID.c_str(), this->ID.c_str(), _sip_server_info->Password.c_str(), "md5",
				www_authenticate_header->realm);
		}
	}
}


void SipDevice::OnRegistrationSuccess(eXosip_event_t* event)
{
	SPDLOG_INFO("注册成功");

	_register_success = true;
	_is_heartbeat_running = true;
	if (_heartbeat_thread == nullptr)
	{
		//如果注册成功，就开始启动心跳线程
		_heartbeat_thread = std::make_shared<std::thread>(&SipDevice::HeartbeatTask, this);
	}
}


void SipDevice::OnMessageNew(eXosip_event_t* event)
{
	if (MSG_IS_MESSAGE(event->request))
	{
		osip_body_t* body = nullptr;
		osip_message_get_body(event->request, 0, &body);
		if (body != nullptr)
		{
			SPDLOG_INFO("request -----> \n {}", body->body);
			SPDLOG_INFO("------------------------------------");
		}
		SendResponseOK(event);

		pugi::xml_document doc;
		auto ret = doc.load_string(body->body);
		if (ret.status != pugi::status_ok)
		{
			SPDLOG_ERROR("load request xml failed");
			return;
		}

		auto root = doc.first_child();
		std::string root_name = root.name();

		if (root_name == "Query")
		{
			OnQueryMessage(doc);
		}
	}
	else if (MSG_IS_BYE(event->request))
	{
		SPDLOG_INFO("Receive Bye");
	}
}


void SipDevice::OnMessageRequestFailed(eXosip_event_t* event)
{
	osip_message_t* register_msg = nullptr;
	auto ret = eXosip_register_build_register(_sip_context, _register_id, 3600, &register_msg);
	if (register_msg == nullptr)
	{
		SPDLOG_ERROR("eXosip_register_build_initial_register failed");
	}
	ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_unlock(_sip_context);

	if (ret == OSIP_SUCCESS)
	{
		SPDLOG_INFO("发送注册信息");
	}
}


void SipDevice::OnCallACK(eXosip_event_t* event)
{
	SPDLOG_INFO("接收到 ACK，开始推流");
	std::scoped_lock<std::mutex> g(_session_mutex);
	auto iter = _session_map.find(event->did);
	if (iter != _session_map.end())
	{
		iter->second->Start();
	}
}


void SipDevice::OnCallClosed(eXosip_event_t* event)
{
	SPDLOG_INFO("接收到 BYE，结束推流");
	std::cout << "close: did: " << event->did << "\n";

	std::scoped_lock<std::mutex> g(_session_mutex);
	auto iter = _session_map.find(event->did);
	if (iter != _session_map.end())
	{
		//向流媒体服务器发送请求，停止发送RTP数据
		iter->second->Stop();
		// 删除此会话
		_session_map.erase(event->did);
	}
}


void SipDevice::OnCallInvite(eXosip_event_t* event)
{
	SPDLOG_INFO("接收到INVITE");
	osip_body_t* sdp_body = nullptr;
	osip_message_get_body(event->request, 0, &sdp_body);
	if (sdp_body == nullptr)
	{
		SPDLOG_ERROR("SDP Error");
		return;
	}
	SPDLOG_INFO("invite -----> \n {}", sdp_body->body);
	sdp_message_t* sdp = NULL;
	if (OSIP_SUCCESS != sdp_message_init(&sdp))
	{
		SPDLOG_ERROR("sdp_message_init failed");
		return;
	}

	//需要根据请求信息来判断具体请求的哪个通道数据
	std::string invite_video_channel_id = "";
	std::shared_ptr<ChannelInfo> channel_info = nullptr;
	if (event->request && event->request->req_uri && event->request->req_uri->username)
	{
		invite_video_channel_id = event->request->req_uri->username;
		SPDLOG_INFO("INVITE: {}", invite_video_channel_id);
		channel_info = FindChannel(invite_video_channel_id);
		if (channel_info == nullptr)
		{
			SPDLOG_ERROR("未找到对应Channel: {}", invite_video_channel_id);
			return;
		}
	}
	else
	{
		SPDLOG_ERROR("event->request->req_uri错误");
		return;
	}

	//SDP内容解析
	auto ret = sdp_message_parse(sdp, sdp_body->body);
	//获取SSRC，y字段数据国标扩展的字段
	std::string ssrc = ParseSSRC(sdp_body->body);
	if (ssrc.empty())
	{
		SPDLOG_ERROR("未找到SSRC");
		return;
	}
	//获取流媒体服务器地址和协议类型
	sdp_connection_t* connect = eXosip_get_video_connection(sdp);
	sdp_media_t* media = eXosip_get_video_media(sdp);

	// 建立一个Session结构，用于保存DialogID，和对应的Channel信息
	auto session = std::make_shared<Session>();
	session->DialogID = event->did;
	session->TargetIP = connect->c_addr;
	session->TargetPort = std::stoi(media->m_port);
	session->UseTcp = strstr(media->m_proto, "TCP");
#ifdef _WIN32
	session->LocalPort = NetHelper::FindAvailablePort(get_port());
#else
	session->LocalPort = get_port();
#endif
	session->SSRC = ssrc;
	session->Channel = channel_info;
	//判断是回放还是实时
	if (strstr(sdp_body->body, "s=Playback"))
	{
		session->Playback = true;
		std::string text = sdp_body->body;
		ParseTimeStr(text, session->StartTime, session->EndTime);
	}
	else
	{
		session->Playback = false;
	}

	SPDLOG_INFO("Session:\n {}", session->ToString());
	{
		std::scoped_lock<std::mutex> g(_session_mutex);
		_session_map.insert({ event->did, session });
	}
	//设备端SDP信息
	std::stringstream ss;
	ss << "v=0\r\n";
	ss << "o={} 0 0 IN IP4 {}\r\n";
	ss << "s={}\r\n";
	ss << "c=IN IP4 {}\r\n";
	ss << "t=0 0\r\n";
	ss << "m=video {} {} 96\r\n";
	ss << "a=sendonly\r\n";
	ss << "a=rtpmap:96 PS/90000\r\n";
	ss << "y={}\r\n";

	auto info = fmt::format(
		ss.str(), invite_video_channel_id, this->IP, session->Playback ? "Playback" : "Play", this->IP,
		session->LocalPort, session->UseTcp ? "TCP/RTP/AVP" : "RTP/AVP", ssrc);

	osip_message_t* message = event->request;
	ret = eXosip_call_build_answer(_sip_context, event->tid, 200, &message);
	if (ret != OSIP_SUCCESS)
	{
		SPDLOG_ERROR("eXosip_call_build_answer failed");
		return;
	}
	osip_message_set_content_type(message, "APPLICATION/SDP");
	osip_message_set_body(message, info.c_str(), info.length());
	eXosip_lock(_sip_context);
	eXosip_message_send_answer(_sip_context, event->tid, 200, message);
	eXosip_unlock(_sip_context);
	SPDLOG_INFO("SDP Response: {}", info);
}


void SipDevice::SendResponseOK(eXosip_event_t* event)
{
	osip_message_t* message = event->request;
	eXosip_message_build_answer(_sip_context, event->tid, 200, &message);

	eXosip_lock(_sip_context);
	eXosip_message_send_answer(_sip_context, event->tid, 200, message);
	eXosip_unlock(_sip_context);
}

/// @brief 心跳
void SipDevice::HeartbeatTask()
{
	while (_is_running && _register_success && _is_heartbeat_running)
	{
		auto text =
			R"(<?xml version="1.0" encoding="GB2312"?>
				<Notify>
					<CmdType>Keepalive</CmdType>
					<SN>{}</SN>
					<DeviceID>{}</DeviceID>
					<Status>OK</Status>
				</Notify>
				)"s;
		auto info = fmt::format(text, get_sn(), this->ID);

		osip_message_t* request = nullptr;
		auto ret = eXosip_message_build_request(
			_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
		if (ret != OSIP_SUCCESS)
		{
			SPDLOG_ERROR("eXosip_message_build_request failed");
			return;
		}

		osip_message_set_content_type(request, "Application/MANSCDP+xml");
		osip_message_set_body(request, info.c_str(), info.length());

		eXosip_lock(_sip_context);
		eXosip_message_send_request(_sip_context, request);
		eXosip_unlock(_sip_context);

		SPDLOG_INFO("Heartbeat");

		std::unique_lock<std::mutex> lck(_heartbeat_mutex);
		_heartbeat_condition.wait_for(lck, std::chrono::seconds(this->HeartbeatInterval));
	}
}


void SipDevice::MobilePositionTask()
{
	while (_is_running && _register_success && _subscription_dialog_id > 0)
	{
		osip_message_t* notify_message = NULL;
		if (OSIP_SUCCESS != eXosip_insubscription_build_notify(_sip_context, _subscription_dialog_id, EXOSIP_SUBCRSTATE_PENDING, EXOSIP_NOTIFY_PENDING, &notify_message))
		{
			SPDLOG_ERROR("eXosip_insubscription_build_notify error");
		}
		else
		{
			std::stringstream ss;
			ss << "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n";
			ss << "<Notify>\r\n";
			ss << "<DeviceID>" << this->ID << "</DeviceID>\r\n";
			ss << "<CmdType>MobilePosition</CmdType>\r\n";
			ss << "<SN>" << get_sn() << "</SN>\r\n";
			ss << "<Time>"
				<< "</Time>\r\n";
			ss << "<Longitude>"
				<< "103.22925"
				<< "</Longitude>\r\n";
			ss << "<Latitude>"
				<< "31.23354 "
				<< "</Latitude>\r\n";
			ss << "<Speed>0.0</Speed>\r\n";
			ss << "<Direction>0.0</Direction>\r\n";
			ss << "<Altitude>500</Altitude>\r\n";
			ss << "</Notify>\r\n";

			auto text = ss.str();
			osip_message_set_content_type(notify_message, "Application/MANSCDP+xml");
			osip_message_set_body(notify_message, text.c_str(), text.length());
			eXosip_insubscription_send_request(_sip_context, _subscription_dialog_id, notify_message);

			SPDLOG_INFO("Insubscription");

			std::unique_lock<std::mutex> lck(_subscription_mutex);
			_subscription_condition.wait_for(lck, std::chrono::seconds(this->HeartbeatInterval));
		}
	}
}


void SipDevice::OnQueryMessage(pugi::xml_document& doc)
{
	std::string response_body = "";
	auto root = doc.first_child();
	auto cmd_node = root.child("CmdType");
	if (cmd_node.empty())
		return;
	std::string cmd = cmd_node.child_value();
	auto sn_node = root.child("SN");
	std::string sn = sn_node.child_value();
	SPDLOG_INFO("SN: {}", sn);
	// 目录查询，返回channels信息
	if (cmd == "Catalog")
	{
		response_body = GenerateCatalogXML(sn);
	}
	// 设备信息查询，发送设备信息,主要包括设备id，设备名称，厂家，版本，channel数量等
	else if (cmd == "DeviceInfo")
	{
		response_body = GenerateDeviceInfoXML(sn);
	}
	// 配置下载
	else if (cmd == "ConfigDownload")
	{
		auto type_node = root.child("ConfigType");
		if (type_node.empty())
			return;
		std::string type = type_node.child_value();
		if (type == "BasicParam")
		{
			response_body = GenerateBasicParamXML(sn);
		}
		if (type == "VideoParamOpt")
		{
			response_body = GenerateVideoParamOptXML(sn);
		}
	}
	// 录像文件查询
	else if (cmd == "RecordInfo")
	{
		std::string device_id = root.child("DeviceID").child_value();
		std::string start_time = root.child("StartTime").child_value();
		std::string end_time = root.child("EndTime").child_value();

		response_body = GenerateRecordInfoXML(sn, device_id, start_time, end_time);
	}

	if (!response_body.empty())
	{
		auto body = FormatXML(response_body);

		osip_message_t* request = nullptr;
		auto ret = eXosip_message_build_request(
			_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
		if (ret != OSIP_SUCCESS)
		{
			SPDLOG_ERROR("eXosip_message_build_request failed");
			return;
		}

		osip_message_set_content_type(request, "Application/MANSCDP+xml");
		osip_message_set_body(request, body.c_str(), body.length());

		eXosip_lock(_sip_context);
		eXosip_message_send_request(_sip_context, request);
		eXosip_unlock(_sip_context);
	}
}


std::string SipDevice::GenerateCatalogXML(const std::string& sn)
{
	auto text =
		R"( <?xml version="1.0" encoding="GB2312"?>
			<Response>
				<CmdType>Catalog</CmdType>
				<SN>{}</SN>
				<DeviceID>{}</DeviceID>
				<SumNum>{}</SumNum>
				<DeviceList Num="{}">
				</DeviceList>
			</Response>
			)"s;

	auto xml = fmt::format(text, sn, this->ID, this->Channels.size(), this->Channels.size());
	pugi::xml_document doc;
	auto ret = doc.load_string(xml.c_str());
	if (ret.status != pugi::status_ok)
	{
		return "";
	}

	//一些设备可能会带有多个通道
	auto device = doc.child("Response").child("DeviceList");
	for (auto&& channel : this->Channels)
	{
		auto node = device.append_child("Item");
		node.append_child("DeviceID").text().set(channel->ID.c_str());
		node.append_child("Name").text().set(channel->Name.c_str());
		node.append_child("Manufacturer").text().set(this->Manufacturer.c_str());
		node.append_child("Model").text().set(this->Name.c_str());
		node.append_child("Status").text().set("ON");
	}

	std::stringstream ss;
	doc.save(ss);

	return ss.str();
}


std::string SipDevice::GenerateDeviceInfoXML(const std::string& sn)
{
	auto text =
		R"( <?xml version="1.0" encoding="GB2312"?>
			<Response>
				<CmdType>DeviceInfo</CmdType>
				<SN>{}</SN>
				<DeviceID>{}</DeviceID>

				<Result>OK</Result>
				<DeviceName>{}</DeviceName>
				<Manufacturer>{}</Manufacturer>

				<Model>SipServer</Model>
				<Firmware>V1.0.0</Firmware>
				<Channel>{}</Channel>
			</Response>
			)"s;

	return fmt::format(text, sn, this->ID, this->Name, this->Manufacturer, this->Channels.size());
}


std::string SipDevice::GenerateVideoParamOptXML(const std::string& sn)
{
	auto text =
		R"( <?xml version="1.0" encoding="GB2312"?>
			<Response>
				<CmdType>ConfigDownload</CmdType>
				<SN>{}</SN>
				<DeviceID>{}</DeviceID>
				<Result>OK</Result>
				<VideoParamOpt>
					<DownloadSpeed>1</DownloadSpeed>
					<Resolution>5/6</Resolution>
				</VideoParamOpt>
			</Response>
			)"s;
	return fmt::format(text, sn, this->ID);
}


std::string SipDevice::GenerateBasicParamXML(const std::string& sn)
{
	auto text =
		R"( <?xml version="1.0" encoding="GB2312"?>
			<Response>
				<CmdType>ConfigDownload</CmdType>
				<SN>{}</SN>
				<DeviceID>{}</DeviceID>

				<Result>OK</Result>
				<BasicParam>
					<Name>{}</Name>
					<DeviceID>{}</DeviceID>
					<SIPServerID>{}</SIPServerID>
					<SIPServerIP>{}</SIPServerIP>
					<SIPServerPort>{}</SIPServerPort>
					<DomainName>{}</DomainName>
					<Expiration>3600</Expiration>
					<Password>{}</Password>
					<HeartBeatInterval>{}</HeartBeatInterval>
					<HeartBeatCount>60</HeartBeatCount>
				</BasicParam>
			</Response>
			)"s;

	return fmt::format(text, sn, this->ID, this->Name, this->ID, _sip_server_info->ID, _sip_server_info->IP,
								_sip_server_info->Port, _sip_server_domain, _sip_server_info->Password, this->HeartbeatInterval);
}



std::string SipDevice::GenerateRecordInfoXML(const std::string& sn,
	const std::string& channel_id, const std::string& start_time, const std::string& end_time)
{
	std::shared_ptr<ChannelInfo> channel_info = FindChannel(channel_id);
	if (channel_info == nullptr) return std::string();
	// 向流媒体服务器发送请求
	std::string response;
	auto ret = HttpClient::GetInstance()->GetMp4RecordInfo(channel_info->Stream,
														   start_time, end_time, response);
	std::string response_body;
	auto text =
		R"( <?xml version="1.0" encoding="GB2312"?>
			<Response>
				<CmdType>RecordInfo</CmdType>
				<SN>{}</SN>
				<DeviceID>{}</DeviceID>
				<Name>{}</Name>
				<SumNum>{}</SumNum>
				<RecordList Num="{}">
				</RecordList>
			</Response>
		)"s;
	try
	{
		pugi::xml_document doc;
		nlohmann::json json_data = nlohmann::json::parse(response);
		size_t record_size = json_data["data"]["fileInfo"].size();
		response_body = fmt::format(text, sn, this->ID, channel_id, record_size, record_size);

		auto doc_ret = doc.load_string(response_body.c_str());
		if (doc_ret.status != pugi::status_ok)
			return std::string();

		auto record_list = doc.child("Response").child("RecordList");

		for (const auto& file_item : json_data["data"]["fileInfo"])
		{
			//文件路径，这里只使用文件名称即可
			std::string file_path = std::filesystem::path(file_item["file_path"].get<std::string>()).filename().u8string();
			std::string str_begin_time = toolkit::CFileTime(file_item["begin_time"]).UTCToLocal().FormatTZ();
			std::string str_end_time = toolkit::CFileTime(file_item["stop_time"]).UTCToLocal().FormatTZ();

			auto node = record_list.append_child("Item");
			node.append_child("DeviceID").text().set(this->ID.c_str());
			node.append_child("Name").text().set(channel_id.c_str());
			node.append_child("FilePath").text().set(file_path.c_str());
			node.append_child("StartTime").text().set(str_begin_time.c_str());
			node.append_child("EndTime").text().set(str_end_time.c_str());
			node.append_child("Address").text().set("");
			node.append_child("Secrecy").text().set("0");
			node.append_child("Type").text().set("time");
		}

		std::stringstream ss;
		doc.save(ss);
		response_body = ss.str();
	}
	catch (const std::exception& ex)
	{
		//查询不到或不存在时，就回复空内容
		response_body = fmt::format(text, sn, this->ID, channel_id, 0, 0);
	}
	return response_body;
}


std::string SipDevice::ParseSSRC(const std::string& text)
{
	auto tokens = nbase::StringTokenize(text.c_str(), "\n");
	for (auto&& token : tokens)
	{
		if (strstr(token.c_str(), "y=0") || strstr(token.c_str(), "y=1"))
		{
			auto result = token.substr(2);
			SPDLOG_INFO("---Y: {}", result);
			return result;
		}
	}

	return std::string();
}


bool SipDevice::ParseTimeStr(std::string& text, std::string& start_time, std::string& end_time)
{
	text = nbase::StringTrim(text);
	auto tokens = nbase::StringTokenize(text.c_str(), "\n");
	for (auto&& token : tokens)
	{
		if (strstr(token.c_str(), "t="))
		{
			start_time = token.substr(2, 11);
			end_time = token.substr(12);

			nbase::StringTrim(start_time);
			nbase::StringTrim(end_time);
			return true;
		}
	}
	return false;
}


std::shared_ptr<ChannelInfo> SipDevice::FindChannel(const std::string& id)
{
	std::shared_ptr<ChannelInfo> channel_info = nullptr;
	for (auto&& channel : this->Channels)
	{
		if (channel->ID.compare(id) == 0)
		{
			channel_info = channel;
			break;
		}
	}

	return channel_info;
}


void SipDevice::OnInSubscriptionNew(eXosip_event_t* event)
{
	SPDLOG_INFO("on_in_subscription_new...");

	osip_message_t* answer = NULL;
	if (OSIP_SUCCESS == eXosip_insubscription_build_answer(_sip_context, event->tid, 200, &answer))
	{
		eXosip_insubscription_send_answer(_sip_context, event->tid, 200, answer);
		_subscription_dialog_id = event->did;

		//	if (_subscription_thread == nullptr) {
		//		_subscription_thread = std::make_shared<std::thread>(&SipDevice::MobilePositionTask, this);
		//	}
	}
}


void SipDevice::OnCallMessageNew(eXosip_event_t* event)
{
	if (!MSG_IS_INFO(event->request))  return;

	auto iter = _session_map.find(event->did);
	if (iter == _session_map.end())
	{
		SendResponseOK(event);
		return;
	}

	auto&& session = iter->second;
	if (!session->Playback)//实时流不处理
	{
		SendResponseOK(event);
		return;
	}

	osip_body_t* body = nullptr;
	osip_message_get_body(event->request, 0, &body);
	if (body == nullptr)
	{
		SPDLOG_ERROR("osip_message_get_body 错误");
		SendResponseOK(event);
		return;
	}

	SPDLOG_INFO("request -----> \n{}", body->body);
	//暂停播放
	if (strstr(body->body, "PAUSE"))
	{
		session->Pause(true);
	}
	else if (strstr(body->body, "PLAY"))
	{
		//播放
		if (strstr(body->body, "Range"))
		{
			session->Pause(false);
		}
		else
		{
			//速度调整
			auto tokens = nbase::StringTokenize(body->body, "\n");
			for (auto&& token : tokens)
			{
				if (strstr(token.c_str(), "Scale"))
				{
					//解析速度
					auto text = token.substr(6);
					nbase::StringTrim(text);
					session->Speed(std::stof(text));
					break;
				}
			}
		}
	}
	else
	{
		SPDLOG_WARN("Not Supported");
	}
	SendResponseOK(event);
}


bool SipDevice::SendStreamFinishedNotify(std::shared_ptr<Session> session)
{
	auto text =
		R"(<?xml version="1.0" encoding="GB2312"?>
			<Notify>
				<CmdType>MediaStatus</CmdType>
				<SN>{}</SN>
				<DeviceID>{}</DeviceID>
				<NotifyType>121</NotifyType>
			</Notify>
			)"s;
	auto info = fmt::format(text, get_sn(), this->ID);

	osip_message_t* request = nullptr;
	auto ret = eXosip_message_build_request(
		_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
	if (ret != OSIP_SUCCESS)
	{
		SPDLOG_ERROR("eXosip_message_build_request failed");
		return false;
	}

	osip_message_set_content_type(request, "Application/MANSCDP+xml");
	osip_message_set_body(request, info.c_str(), info.length());

	eXosip_lock(_sip_context);
	eXosip_message_send_request(_sip_context, request);
	eXosip_unlock(_sip_context);

	return true;
}

std::string SipDevice::FormatXML(const std::string& xml)
{
	pugi::xml_document doc;
	auto ret = doc.load_string(xml.c_str());
	if (ret.status != pugi::status_ok)
	{
		return xml;
	}

	std::stringstream ss;
	doc.save(ss);

	return ss.str();
}


void SipDevice::OnStreamChanedCallback(const std::string& app, const std::string& stream, bool regist)
{
	//只需要判断录像回放的流注销事件即可
	if (!regist && app == "record")
	{
		std::scoped_lock<std::mutex> g(_session_mutex);
		for (auto&& [id, session] : _session_map)
		{
			if (session->SSRC == stream)
			{
				SendStreamFinishedNotify(session);

				//从map中删除此session，流注销已自动释放
				_session_map.erase(id);
				break;
			}
		}
	}
}


/// @brief 开始播放。接收到ACK消息后，让流媒体服务器向指定地址发送视频数据。
void Session::Start()
{
	if (this->Playback)
	{
		// 录像回放：向流媒体服务器发送回放请求，需要指定收发数据的端口，SSRC和时间范围
		auto ret = HttpClient::GetInstance()->StartSendPlaybackRtp(
			this->Channel, this->SSRC, this->TargetIP, this->TargetPort, this->LocalPort,
			this->StartTime, this->EndTime, this->UseTcp);
	}
	else
	{
		// 实时流：向流媒体服务器发送实时推送请求，需要指定收发数据的端口，SSRC和时间范围
		auto ret = HttpClient::GetInstance()->StartSendRtp(
			this->Channel, this->SSRC, this->TargetIP, this->TargetPort, this->LocalPort, this->UseTcp);
	}
}

/// @brief 停止播放，通过app和stream来区分是哪个流。
void Session::Stop()
{
	HttpClient::GetInstance()->StopSendRtp(
		this->Playback ? "record" : this->Channel->App,
		this->Playback ? this->SSRC : this->Channel->Stream);
}


// 向流媒体服务器发送暂停请求
void Session::Pause(bool flag)
{
	auto ret = HttpClient::GetInstance()->SetPause("record", this->SSRC, flag);
}

void Session::Seek(int64_t pos)
{
}

// 设置媒体流速度
void Session::Speed(float speed)
{
	auto ret = HttpClient::GetInstance()->SetSpeed("record", this->SSRC, speed);
}
