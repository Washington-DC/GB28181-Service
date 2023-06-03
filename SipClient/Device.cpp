#include "pch.h"
#include "Device.h"
#include "HttpClient.h"
#include "NetUtils.h"

static int start_port = 20000;
static int SN_MAX = 99999999;
static int sn = 0;

static int get_port() {
	start_port++;
	if (start_port > 65500)
	{
		return 20000;
	}
	return start_port;
}
static int get_sn() {
	if (sn >= SN_MAX) {
		sn = 0;
	}
	sn++;
	return sn;
}

SipDevice::SipDevice(const std::shared_ptr<DeviceInfo> info, std::shared_ptr<SipServerInfo> sip_server_info) {
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

bool SipDevice::init() {
	_sip_context = eXosip_malloc();
	if (OSIP_SUCCESS != eXosip_init(_sip_context)) {
		LOG(ERROR) << "eXosip_init failed";
		return false;
	}
	eXosip_set_option(_sip_context, EXOSIP_OPT_SET_HEADER_USER_AGENT, this->Manufacturer.c_str());

	_from_uri = fmt::format("sip:{}@{}:{}", this->ID, this->IP, this->Port);
	_contact_url = fmt::format("sip:{}@{}:{}", this->ID, this->IP, this->Port);
	_proxy_uri = fmt::format("sip:{}@{}:{}", _sip_server_info->ID, _sip_server_info->IP, _sip_server_info->Port);

	return true;
}

/// @brief 启动sip客户端功能
/// 绑定本地端口，创建线程接收数据，发送注册信息
/// @return 
bool SipDevice::start_sip_client() {
	if (NetHelper::IsPortAvailable(this->Port))
		_local_port = this->Port;
	else {
		_local_port = NetHelper::FindAvailablePort();
		LOG(INFO) << fmt::format("端口:{}被占用，将使用端口：{}", this->Port, _local_port);
	}

	if (OSIP_SUCCESS != eXosip_listen_addr(_sip_context, this->Protocol, NULL, _local_port, AF_INET, 0)) {
		LOG(ERROR) << "eXosip_listen_addr";
		eXosip_quit(_sip_context);
		return false;
	}

	_is_running = true;
	_sip_thread = std::make_shared<std::thread>(&SipDevice::on_response, this);

	eXosip_clear_authentication_info(_sip_context);

	osip_message_t* register_msg = nullptr;
	_register_id = eXosip_register_build_initial_register(
		_sip_context, _from_uri.c_str(), _proxy_uri.c_str(), _contact_url.c_str(), 3600, &register_msg);
	if (register_msg == nullptr) {
		LOG(ERROR) << "eXosip_register_build_initial_register failed";
	}

	eXosip_lock(_sip_context);
	auto ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_unlock(_sip_context);

	if (ret == OSIP_SUCCESS) {
		LOG(INFO) << "发送注册信息";
		return true;
	}
	else {
		LOG(ERROR) << "发送注册信息失败";
		return false;
	}
}

/// @brief 停止sip功能
/// @return 
bool SipDevice::stop_sip_client() {
	_is_running = false;

	//退出时，停止推送所有数据
	for (auto&& [id, session] : _session_map)
	{
		HttpClient::GetInstance()->StopSendRtp(session->Channel);
	}

	if (_sip_thread) {
		_sip_thread->join();
		_sip_thread = nullptr;
	}

	if (_sip_context) {
		eXosip_quit(_sip_context);
		_sip_context = nullptr;
	}

	_register_success = false;
	_is_heartbeat_running = false;

	if (_heartbeat_thread) {
		_heartbeat_condition.notify_one();
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}

	return true;
}

/// @brief 注销
/// @return 
bool SipDevice::log_out() {
	_is_heartbeat_running = false;

	if (_heartbeat_thread) {
		_heartbeat_condition.notify_one();
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}
	_logout = true;

	LOG(INFO) << "发送注销命令";
	osip_message_t* register_msg = nullptr;
	auto ret = eXosip_register_build_register(_sip_context, _register_id, 0, &register_msg);
	if (ret != OSIP_SUCCESS) {
		LOG(ERROR) << "eXosip_register_build_initial_register failed";
		return false;
	}

	osip_contact_t* contact = nullptr;
	osip_message_get_contact(register_msg, 0, &contact);

	auto info = fmt::format("{};expires=0", _contact_url);
	osip_list_remove(&register_msg->contacts, 0);
	osip_message_set_contact(register_msg, info.c_str());
	osip_message_set_header(register_msg, "Logout-Reason", "logout");

	// OSIP_BADPARAMETER
	eXosip_lock(_sip_context);
	ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_clear_authentication_info(_sip_context);
	eXosip_unlock(_sip_context);

	return true;
}

/// @brief 轮询接收数据
void SipDevice::on_response() {
	eXosip_event_t* event = nullptr;
	while (_is_running) {
		event = eXosip_event_wait(_sip_context, 0, 50);
		if (event) {
			LOG(INFO) << "------------- Receive: " << magic_enum::enum_name<eXosip_event_type>(event->type);
		}

		if (!_logout) {
			eXosip_lock(_sip_context);
			eXosip_automatic_action(_sip_context);
			eXosip_unlock(_sip_context);
		}

		if (event == nullptr)
			continue;

		switch (event->type) {
			case EXOSIP_IN_SUBSCRIPTION_NEW:
				break;
			case EXOSIP_MESSAGE_NEW: //查询目录
				on_message_new(event);
				break;
			case EXOSIP_REGISTRATION_SUCCESS: //注册成功
				on_registration_success(event);
				break;
			case EXOSIP_REGISTRATION_FAILURE: //注册失败
				on_registration_failure(event);
				break;
			case EXOSIP_MESSAGE_REQUESTFAILURE: //  服务端删除此设备后，发送请求会收到此回复
				on_message_request_failure(event);
				break;
			case EXOSIP_CALL_ACK: //开始推流
				on_call_ack(event);
				break;
			case EXOSIP_CALL_CLOSED: //关闭推流
				on_call_closed(event);
				break;
			case EXOSIP_CALL_INVITE: //请求会话
				on_call_invite(event);
				break;
			default:
				break;
		}

		if (event != nullptr) {
			eXosip_event_free(event);
			event = nullptr;
		}
	}
}

/// @brief 注册失败
/// @param event 
void SipDevice::on_registration_failure(eXosip_event_t* event) {
	LOG(ERROR) << "注册失败";
	_register_success = false;
	_is_heartbeat_running = false;
	if (event->response == nullptr) {
		return;
	}
	LOG(ERROR) << "注册失败: " << event->response->status_code;
	_is_heartbeat_running = false;
	if (_heartbeat_thread) {
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}

	if (event->response->status_code == 401 || event->response->status_code == 403) {
		osip_www_authenticate_t* www_authenticate_header = nullptr;
		osip_message_get_www_authenticate(event->response, 0, &www_authenticate_header);
		if (www_authenticate_header)
		{
			_sip_server_domain = osip_strdup_without_quote(www_authenticate_header->realm);
			eXosip_add_authentication_info(
				_sip_context, this->ID.c_str(), this->ID.c_str(), _sip_server_info->Password.c_str(), "md5",
				www_authenticate_header->realm);
		}
	}
}

/// @brief 注册成功
/// @param event 
void SipDevice::on_registration_success(eXosip_event_t* event) {
	LOG(INFO) << "注册成功";

	if (!_logout) {
		_register_success = true;
		_is_heartbeat_running = true;
		if (_heartbeat_thread == nullptr) {
			_heartbeat_thread = std::make_shared<std::thread>(&SipDevice::heartbeat_task, this);
		}
	}
}

/// @brief 收到消息，这里主要有查询Catalog和DeviceInfo两种
/// @param event 
void SipDevice::on_message_new(eXosip_event_t* event) {
	if (MSG_IS_MESSAGE(event->request)) {
		osip_body_t* body = nullptr;
		osip_message_get_body(event->request, 0, &body);
		if (body != nullptr) {
			LOG(INFO) << "request -----> \n" << body->body;
			LOG(INFO) << "------------------------------------";
		}
		send_response_ok(event);

		pugi::xml_document doc;
		auto ret = doc.load_string(body->body);
		if (ret.status != pugi::status_ok) {
			LOG(ERROR) << "load request xml failed";
			return;
		}

		auto root = doc.first_child();
		std::string root_name = root.name();

		if (root_name == "Query") {
			auto cmd_node = root.child("CmdType");
			if (cmd_node.empty())
				return;
			std::string cmd = cmd_node.child_value();
			auto sn_node = root.child("SN");
			std::string sn = sn_node.child_value();
			LOG(INFO) << "SN: " << sn;
			//目录查询，返回channels信息
			if (cmd == "Catalog") {
				auto text = generate_catalog_xml(sn);

				osip_message_t* request = nullptr;
				auto ret = eXosip_message_build_request(
					_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
				if (ret != OSIP_SUCCESS) {
					LOG(ERROR) << "eXosip_message_build_request failed";
					return;
				}

				osip_message_set_content_type(request, "Application/MANSCDP+xml");
				osip_message_set_body(request, text.c_str(), text.length());

				eXosip_lock(_sip_context);
				eXosip_message_send_request(_sip_context, request);
				eXosip_unlock(_sip_context);
			}
			//设备信息查询，发送设备信息,主要包括设备id，设备名称，厂家，版本，channel数量等
			else if (cmd == "DeviceInfo") {
				auto text =
					R"(<?xml version="1.0" encoding="GB2312"?>
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

				auto xml = fmt::format(text, sn, this->ID, this->Name, this->Manufacturer, this->Channels.size());
				osip_message_t* request = nullptr;
				auto ret = eXosip_message_build_request(
					_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
				if (ret != OSIP_SUCCESS) {
					LOG(ERROR) << "eXosip_message_build_request failed";
					return;
				}

				osip_message_set_content_type(request, "Application/MANSCDP+xml");
				osip_message_set_body(request, xml.c_str(), xml.length());

				eXosip_lock(_sip_context);
				eXosip_message_send_request(_sip_context, request);
				eXosip_unlock(_sip_context);
			}
			//配置下载
			else if (cmd == "ConfigDownload") {
				auto type_node = root.child("ConfigType");
				if (type_node.empty())
					return;
				std::string type = type_node.child_value();
				if (type == "BasicParam")
				{
					auto text =
						R"(<?xml version="1.0" encoding="GB2312"?>
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

					auto xml = fmt::format(text, sn, this->ID, this->Name, this->ID, _sip_server_info->ID, _sip_server_info->IP,
						_sip_server_info->Port, _sip_server_domain, _sip_server_info->Password, this->HeartbeatInterval);

					osip_message_t* request = nullptr;
					auto ret = eXosip_message_build_request(
						_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
					if (ret != OSIP_SUCCESS) {
						LOG(ERROR) << "eXosip_message_build_request failed";
						return;
					}

					osip_message_set_content_type(request, "Application/MANSCDP+xml");
					osip_message_set_body(request, xml.c_str(), xml.length());

					eXosip_lock(_sip_context);
					eXosip_message_send_request(_sip_context, request);
					eXosip_unlock(_sip_context);
				}

				if (type == "VideoParamOpt")
				{
					auto text =
						R"(<?xml version="1.0" encoding="GB2312"?>
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
					auto xml = fmt::format(text, sn, this->ID);

					osip_message_t* request = nullptr;
					auto ret = eXosip_message_build_request(
						_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
					if (ret != OSIP_SUCCESS) {
						LOG(ERROR) << "eXosip_message_build_request failed";
						return;
					}

					osip_message_set_content_type(request, "Application/MANSCDP+xml");
					osip_message_set_body(request, xml.c_str(), xml.length());

					eXosip_lock(_sip_context);
					eXosip_message_send_request(_sip_context, request);
					eXosip_unlock(_sip_context);
				}

			}
			else if (cmd == "RecordInfo") {
			}
		}
	}
	else if (MSG_IS_BYE(event->request)) {
		LOG(INFO) << "Receive Bye";
	}
}

/// @brief 服务端删除设备后，可能会收到此信息，此时重新发送注册信息
/// @param event 
void SipDevice::on_message_request_failure(eXosip_event_t* event) {
	osip_message_t* register_msg = nullptr;
	auto ret = eXosip_register_build_register(_sip_context, _register_id, 3600, &register_msg);
	if (register_msg == nullptr) {
		LOG(ERROR) << "eXosip_register_build_initial_register failed";
	}

	_is_running = true;

	eXosip_lock(_sip_context);
	ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_unlock(_sip_context);

	if (ret == OSIP_SUCCESS) {
		LOG(INFO) << "发送注册信息";
	}
}

/// @brief 开始推流
/// @param event 
void SipDevice::on_call_ack(eXosip_event_t* event) {
	LOG(INFO) << "接收到 ACK，开始推流";

	std::shared_ptr<SessinInfo> session = nullptr;
	if (event->request && event->request->req_uri && event->request->req_uri->username) {
		auto invite_video_channel_id = event->request->req_uri->username;
		if (_session_map.find(invite_video_channel_id) != _session_map.end()) {
			session = _session_map[invite_video_channel_id];

			// TODO  向流媒体服务器发送请求，开始向指定地址发送RTP数据
			auto ret = HttpClient::GetInstance()->StartSendRtp(
				session->Channel, session->SSRC, session->TargetIP, session->TargetPort, session->LocalPort,
				session->UseTcp);
		}
	}
}

/// @brief 停止推流
/// @param event 
void SipDevice::on_call_closed(eXosip_event_t* event) {
	LOG(INFO) << "接收到 BYE，结束推流";

	std::shared_ptr<SessinInfo> session = nullptr;
	if (event->request && event->request->req_uri && event->request->req_uri->username) {
		auto invite_video_channel_id = event->request->req_uri->username;
		auto iter = _session_map.find(invite_video_channel_id);
		if (iter != _session_map.end()) {
			session = _session_map[invite_video_channel_id];

			// TODO  向流媒体服务器发送请求，停止发送RTP数据
			auto ret = HttpClient::GetInstance()->StopSendRtp(session->Channel);

			_session_map.erase(iter);
		}
	}
}

/// @brief 请求会话，发送sdp信息
/// @param event 
void SipDevice::on_call_invite(eXosip_event_t* event) {
	LOG(INFO) << "接收到INVITE";
	osip_body_t* sdp_body = nullptr;
	osip_message_get_body(event->request, 0, &sdp_body);
	if (sdp_body != nullptr) {
		LOG(INFO) << "request -----> \n" << sdp_body->body;
		LOG(INFO) << "------------------------------------";
	}
	else {
		LOG(ERROR) << "SDP Error";
		return;
	}

	sdp_message_t* sdp = NULL;
	if (OSIP_SUCCESS != sdp_message_init(&sdp)) {
		LOG(ERROR) << "sdp_message_init failed";
		return;
	}

	std::string invite_video_channel_id = "";
	std::shared_ptr<ChannelInfo> channel_info = nullptr;
	if (event->request && event->request->req_uri && event->request->req_uri->username) {
		invite_video_channel_id = event->request->req_uri->username;
		LOG(INFO) << "INVITE: " << invite_video_channel_id;
		channel_info = find_channel(invite_video_channel_id);
	}
	else
	{
		LOG(ERROR) << "event->request->req_uri错误";
		return;
	}

	if (channel_info == nullptr) {
		LOG(ERROR) << "未找到对应Channel: " << invite_video_channel_id;
		return;
	}

	auto ret = sdp_message_parse(sdp, sdp_body->body);
	std::string ssrc = "";
	{
		std::string text = sdp_body->body;
		ssrc = parse_ssrc(text);
		if (ssrc.empty())
		{
			LOG(ERROR) << "未找到SSRC";
			return;
		}
	}
	sdp_connection_t* connect = eXosip_get_video_connection(sdp);
	sdp_media_t* media = eXosip_get_video_media(sdp);

	auto session = std::make_shared<SessinInfo>();
	session->TargetIP = connect->c_addr;
	session->TargetPort = std::stoi(media->m_port);
	auto protocol = media->m_proto;
	session->UseTcp = strstr(protocol, "TCP");
	session->LocalPort = NetHelper::FindAvailablePort(get_port());
	session->SSRC = ssrc;
	session->ID = invite_video_channel_id;
	session->Channel = channel_info;

	LOG(INFO) << "Session:\n" << session->to_string();
	_session_map.insert({ invite_video_channel_id, session });

	std::stringstream ss;
	ss << "v=0\r\n";
	ss << "o={} 0 0 IN IP4 {}\r\n";
	ss << "s=Play\r\n";
	ss << "c=IN IP4 {}\r\n";
	ss << "t=0 0\r\n";
	ss << "m=video {} {} 96\r\n";
	ss << "a=sendonly\r\n";
	ss << "a=rtpmap:96 PS/90000\r\n";
	ss << "y={}\r\n";

	auto info = fmt::format(
		ss.str(), invite_video_channel_id, this->IP, this->IP, session->LocalPort,
		session->UseTcp ? "TCP/RTP/AVP" : "RTP/AVP", ssrc);

	osip_message_t* message = event->request;
	ret = eXosip_call_build_answer(_sip_context, event->tid, 200, &message);
	if (ret != OSIP_SUCCESS) {
		LOG(ERROR) << "eXosip_call_build_answer failed";
		return;
	}
	osip_message_set_content_type(message, "APPLICATION/SDP");
	osip_message_set_body(message, info.c_str(), info.length());
	eXosip_call_send_answer(_sip_context, event->tid, 200, message);

	LOG(INFO) << "SDP Response: " << info;
}

/// @brief 返回成功消息
/// @param event 
void SipDevice::send_response_ok(eXosip_event_t* event) {
	osip_message_t* message = event->request;
	eXosip_message_build_answer(_sip_context, event->tid, 200, &message);

	eXosip_lock(_sip_context);
	eXosip_message_send_answer(_sip_context, event->tid, 200, message);
	eXosip_unlock(_sip_context);
}

/// @brief 心跳
void SipDevice::heartbeat_task() {
	while (_is_running && _register_success && _is_heartbeat_running) {
		auto text =
			R"(<?xml version="1.0"?>
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
		if (ret != OSIP_SUCCESS) {
			LOG(ERROR) << "eXosip_message_build_request failed";
			return;
		}

		osip_message_set_content_type(request, "Application/MANSCDP+xml");
		osip_message_set_body(request, info.c_str(), info.length());

		eXosip_lock(_sip_context);
		eXosip_message_send_request(_sip_context, request);
		eXosip_unlock(_sip_context);

		LOG(INFO) << "Heartbeat";

		std::unique_lock<std::mutex> lck(_heartbeat_mutex);
		_heartbeat_condition.wait_for(lck, std::chrono::seconds(this->HeartbeatInterval));
	}
}

/// @brief 生成目录信息
/// @param sn 
/// @return 
std::string SipDevice::generate_catalog_xml(const std::string& sn) {
	auto text =
		R"(<?xml version="1.0" encoding="GB2312"?>
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
	if (ret.status != pugi::status_ok) {
		return "";
	}

	auto device = doc.child("Response").child("DeviceList");
	for (auto&& channel : this->Channels) {
		auto node = device.append_child("Item");
		node.append_child("DeviceID").text().set(channel->ID.c_str());
		node.append_child("Name").text().set(channel->Name.c_str());
		node.append_child("Manufacturer").text().set(this->Manufacturer.c_str());
		node.append_child("Model").text().set(this->Name.c_str());
		node.append_child("Status").text().set("ON");
	}

	std::stringstream ss;
	doc.save(ss);
	//LOG(INFO) << "Catalog:----------------------------- \n" << ss.str();

	return ss.str();
}

std::string SipDevice::parse_ssrc(std::string text) {
	text = nbase::StringTrim(text);
	auto tokens = nbase::StringTokenize(text.c_str(), "\n");
	for (auto&& token : tokens)
	{
		if (strstr(token.c_str(), "y=0")) {
			auto result = token.substr(2);
			LOG(INFO) << "---Y: " << result;
			return result;
		}
	}

	return std::string();
}

std::shared_ptr<ChannelInfo> SipDevice::find_channel(std::string id) {
	std::shared_ptr<ChannelInfo> channel_info = nullptr;
	for (auto&& channel : this->Channels) {
		if (channel->ID.compare(id) == 0) {
			channel_info = channel;
			break;
		}
	}

	return channel_info;
}
