#include "pch.h"
#include "Device.h"
#include "NetUtils.h"

static int start_port = 40000;
static int SN_MAX = 99999999;
static int sn = 0;

static int get_port() {
	start_port++;
	return start_port;
}
static int get_sn() {
	if (sn >= SN_MAX) {
		sn = 0;
	}
	sn++;
	return sn;
}


SipDevice::SipDevice(const std::shared_ptr<DeviceInfo> info,
					 std::shared_ptr<SipServerInfo> sip_server_info,
					 std::shared_ptr<MediaServerInfo> media_server_info)
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
	this->_media_server_info = media_server_info;
}


bool SipDevice::init()
{
	_sip_context = eXosip_malloc();
	if (OSIP_SUCCESS != eXosip_init(_sip_context))
	{
		LOG(ERROR) << "eXosip_init failed";
		return false;
	}

	_from_uri = fmt::format("sip:{}@{}:{}", this->ID, this->IP, this->Port);
	_contact_url = fmt::format("sip:{}@{}:{}", this->ID, this->IP, this->Port);
	_proxy_uri = fmt::format("sip:{}@{}:{}", _sip_server_info->ID, _sip_server_info->IP, _sip_server_info->Port);

	_baseurl = fmt::format(L"http://{}:{}", nbase::win32::MBCSToUnicode(_media_server_info->IP), _media_server_info->Port);

	return true;
}


bool SipDevice::start_sip_client()
{
	if (NetHelper::IsPortAvailable(this->Port))
		_local_port = this->Port;
	else
	{
		_local_port = NetHelper::FindAvailablePort();
		LOG(INFO) << fmt::format("端口:{}被占用，将使用端口：{}", this->Port, _local_port);
	}

	if (OSIP_SUCCESS != eXosip_listen_addr(_sip_context, this->Protocol, NULL, _local_port, AF_INET, 0))
	{
		LOG(ERROR) << "eXosip_listen_addr";
		eXosip_quit(_sip_context);
		return false;
	}

	eXosip_clear_authentication_info(_sip_context);

	osip_message_t* register_msg = nullptr;
	_register_id = eXosip_register_build_initial_register(_sip_context, _from_uri.c_str(), _proxy_uri.c_str(), _contact_url.c_str(), 3600, &register_msg);
	if (register_msg == nullptr)
	{
		LOG(ERROR) << "eXosip_register_build_initial_register failed";
	}

	_is_running = true;
	_sip_thread = std::make_shared<std::thread>(&SipDevice::on_response, this);
	eXosip_lock(_sip_context);
	auto ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_unlock(_sip_context);

	if (ret == OSIP_SUCCESS)
	{
		LOG(INFO) << "发送注册信息";
		return true;
	}
	else
	{
		LOG(ERROR) << "发送注册信息失败";
		return false;
	}
}


bool SipDevice::stop_sip_client()
{
	_is_running = false;

	if (_is_pushing_stream)
	{
		on_call_closed(nullptr);
		_is_pushing_stream = false;
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

	_register_success = false;
	_is_heartbeat_running = false;

	if (_heartbeat_thread)
	{
		_heartbeat_condition.notify_one();
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}

	return true;
}

bool SipDevice::send_device_info()
{
	auto text = R"(<?xml version="1.0" encoding="GB2312"?>
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

	auto xml = fmt::format(text, get_sn(), this->ID, this->Name, this->Manufacturer, this->Channels.size());
	osip_message_t* request = nullptr;
	auto ret = eXosip_message_build_request(_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
	if (ret != OSIP_SUCCESS)
	{
		LOG(ERROR) << "eXosip_message_build_request failed";
		return false;
	}

	osip_message_set_content_type(request, "Application/MANSCDP+xml");
	osip_message_set_body(request, xml.c_str(), xml.length());

	eXosip_lock(_sip_context);
	eXosip_message_send_request(_sip_context, request);
	eXosip_unlock(_sip_context);

	return true;
}

bool SipDevice::log_out()
{
	_is_heartbeat_running = false;

	if (_heartbeat_thread)
	{
		_heartbeat_condition.notify_one();
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}
	_logout = true;;

	LOG(INFO) << "发送注销命令";
	osip_message_t* register_msg = nullptr;
	auto ret = eXosip_register_build_register(_sip_context, _register_id, 0, &register_msg);
	if (ret != OSIP_SUCCESS)
	{
		LOG(ERROR) << "eXosip_register_build_initial_register failed";
		return false;
	}

	osip_contact_t* contact = nullptr;
	osip_message_get_contact(register_msg, 0, &contact);

	auto info = fmt::format("{};expires=0", _contact_url);
	osip_list_remove(&register_msg->contacts, 0);
	osip_message_set_contact(register_msg, info.c_str());
	osip_message_set_header(register_msg, "Logout-Reason", "logout");

	//OSIP_BADPARAMETER
	eXosip_lock(_sip_context);
	ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_clear_authentication_info(_sip_context);
	eXosip_unlock(_sip_context);


	return true;
}

void SipDevice::on_response()
{
	eXosip_event_t* event = nullptr;
	while (_is_running)
	{
		event = eXosip_event_wait(_sip_context, 0, 50);
		if (event)
		{
			LOG(INFO) << "------------- Receive: " << magic_enum::enum_name<eXosip_event_type>(event->type);
		}

		if (!_logout)
		{
			eXosip_lock(_sip_context);
			eXosip_automatic_action(_sip_context);
			eXosip_unlock(_sip_context);
		}

		if (event == nullptr)
			continue;

		switch (event->type)
		{
			case EXOSIP_IN_SUBSCRIPTION_NEW:
				break;
			case EXOSIP_MESSAGE_NEW:  //查询目录
				on_message_new(event);
				break;
			case EXOSIP_REGISTRATION_SUCCESS:  //注册成功
				on_registration_success(event);
				break;
			case EXOSIP_REGISTRATION_FAILURE:  //注册失败
				on_registration_failure(event);
				break;
			case EXOSIP_MESSAGE_REQUESTFAILURE://  服务端删除此设备后，发送请求会收到此回复
				on_message_request_failure(event);
				break;
			case EXOSIP_CALL_ACK://开始推流
				on_call_ack(event);
				break;
			case EXOSIP_CALL_CLOSED://关闭推流
				on_call_closed(event);
				break;
			case EXOSIP_CALL_INVITE:
				on_call_invite(event);
				break;
			default:
				break;
		}

		if (event != nullptr)
		{
			eXosip_event_free(event);
			event = nullptr;
		}
	}
}

void SipDevice::on_registration_failure(eXosip_event_t* event)
{
	LOG(ERROR) << "注册失败";
	_register_success = false;
	_is_heartbeat_running = false;
	if (event->response == nullptr)
	{
		return;
	}
	LOG(ERROR) << "注册失败: " << event->response->status_code;
	_is_heartbeat_running = false;
	if (_heartbeat_thread)
	{
		_heartbeat_thread->join();
		_heartbeat_thread = nullptr;
	}

	if (event->response->status_code == 401)
	{
		osip_www_authenticate_t* www_authenticate_header = nullptr;
		osip_message_get_www_authenticate(event->response, 0, &www_authenticate_header);
		eXosip_add_authentication_info(_sip_context, this->ID.c_str(), this->ID.c_str(), _sip_server_info->Password.c_str(), "md5", www_authenticate_header->realm);
	}
}



void SipDevice::on_registration_success(eXosip_event_t* event)
{
	LOG(INFO) << "注册成功";

	if (!_logout)
	{
		_register_success = true;
		_is_heartbeat_running = true;
		if (_heartbeat_thread)
		{
			_is_heartbeat_running = true;
		}
		else
		{
			_heartbeat_thread = std::make_shared<std::thread>(&SipDevice::heartbeat_task, this);
		}
	}

	//send_device_info();
}

void SipDevice::on_message_new(eXosip_event_t* event)
{
	if (MSG_IS_MESSAGE(event->request))
	{
		osip_body_t* body = nullptr;
		osip_message_get_body(event->request, 0, &body);
		if (body != nullptr)
		{
			LOG(INFO) << "request -----> \n" << body->body;
			LOG(INFO) << "------------------------------------";
		}
		send_response_ok(event);

		pugi::xml_document doc;
		if (!doc.load_string(body->body))
		{
			LOG(ERROR) << "load request xml failed";
			return;
		}

		auto root = doc.first_child();
		std::string root_name = root.name();

		if (root_name == "Query")
		{
			auto cmd_node = root.child("CmdType");
			if (cmd_node.empty())
				return;
			std::string cmd = cmd_node.child_value();
			auto sn_node = root.child("SN");
			std::string sn = sn_node.child_value();
			LOG(INFO) << "SN: " << sn;
			if (cmd == "Catalog")
			{
				auto text = generate_catalog_xml(sn);

				osip_message_t* request = nullptr;
				auto ret = eXosip_message_build_request(_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
				if (ret != OSIP_SUCCESS)
				{
					LOG(ERROR) << "eXosip_message_build_request failed";
					return;
				}

				osip_message_set_content_type(request, "Application/MANSCDP+xml");
				osip_message_set_body(request, text.c_str(), text.length());

				eXosip_lock(_sip_context);
				eXosip_message_send_request(_sip_context, request);
				eXosip_unlock(_sip_context);
			}
			else if (cmd == "DeviceInfo")
			{
				auto text = R"(<?xml version="1.0" encoding="GB2312"?>
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
				auto ret = eXosip_message_build_request(_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
				if (ret != OSIP_SUCCESS)
				{
					LOG(ERROR) << "eXosip_message_build_request failed";
					return;
				}

				osip_message_set_content_type(request, "Application/MANSCDP+xml");
				osip_message_set_body(request, xml.c_str(), xml.length());

				eXosip_lock(_sip_context);
				eXosip_message_send_request(_sip_context, request);
				eXosip_unlock(_sip_context);
			}
			else if (cmd == "RecordInfo")
			{

			}
		}
	}
	else if (MSG_IS_BYE(event->request))
	{
		LOG(INFO) << "Receive Bye";
	}
}

void SipDevice::on_message_request_failure(eXosip_event_t* event)
{
	osip_message_t* register_msg = nullptr;
	auto ret = eXosip_register_build_register(_sip_context, _register_id, 3600, &register_msg);
	if (register_msg == nullptr)
	{
		LOG(ERROR) << "eXosip_register_build_initial_register failed";
	}

	_is_running = true;

	eXosip_lock(_sip_context);
	ret = eXosip_register_send_register(_sip_context, _register_id, register_msg);
	eXosip_unlock(_sip_context);

	if (ret == OSIP_SUCCESS)
	{
		LOG(INFO) << "发送注册信息";
	}

}

void SipDevice::on_call_ack(eXosip_event_t* event)
{
	LOG(INFO) << "接收到 ACK，开始推流";

	_call_id = event->cid;
	_dialog_id = event->did;

	//TODO  向流媒体服务器发送请求，开始向指定地址发送RTP数据
	stringstreambuf buffer;
	pplx::task<void> request_task = buffer.sync()
		.then([this]()
			  {
				  http_client_config config;
				  config.set_timeout(1000ms);
				  http_client client(_baseurl, config);
				  uri_builder builder(L"/index/api/startSendRtp");

				  builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_media_server_info->Secret));
				  builder.append_query(L"vhost", L"__defaultVhost__");
				  builder.append_query(L"app", L"h265");
				  builder.append_query(L"stream", L"ch2/sub/av_stream");
				  builder.append_query(L"ssrc", nbase::win32::MBCSToUnicode(_ssrc));
				  builder.append_query(L"dst_url", nbase::win32::MBCSToUnicode(_target_ip));
				  builder.append_query(L"dst_port", std::to_wstring(_target_port));
				  builder.append_query(L"is_udp", _use_tcp ? L"0" : L"1");
				  builder.append_query(L"src_port", std::to_wstring(_listen_port));

				  LOG(INFO) << "向流媒体服务器发送请求: \n" << nbase::win32::UnicodeToMBCS(builder.to_string());

				  return client.request(methods::GET, builder.to_string());
			  })
		.then([this, buffer](http_response response)
			  {
				  return response.body().read_to_end(buffer);
			  })
				  .then([this, buffer](size_t size)
						{
							LOG(INFO) << "流媒体服务器返回: " << buffer.collection();

							_is_pushing_stream = true;
						});

			  try
			  {
				  request_task.wait();
			  }
			  catch (const std::exception& e)
			  {
				  auto text = e.what();
				  LOG(ERROR) << e.what();
			  }
}

void SipDevice::on_call_closed(eXosip_event_t* event)
{
	LOG(INFO) << "接收到 BYE，结束推流";

	_call_id = -1;
	_dialog_id = -1;

	//TODO  向流媒体服务器发送请求，停止发送RTP数据
	stringstreambuf buffer;
	pplx::task<void> request_task = buffer
		.sync()
		.then([this]()
			  {
				  http_client_config config;
				  config.set_timeout(500ms);
				  http_client client(_baseurl, config);
				  uri_builder builder(L"/index/api/stopSendRtp");
				  builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_media_server_info->Secret));
				  builder.append_query(L"vhost", L"__defaultVhost__");
				  builder.append_query(L"app", L"h265");
				  builder.append_query(L"stream", L"ch2/sub/av_stream");

				  return client.request(methods::GET, builder.to_string());
			  })
		.then([this, buffer](http_response response)
			  {
				  return response.body().read_to_end(buffer);
			  })
				  .then([this, buffer](size_t size)
						{
							LOG(INFO) << "流媒体服务器返回: " << buffer.collection();
							_is_pushing_stream = false;
						});

			  try
			  {
				  request_task.wait();
			  }
			  catch (const std::exception& e)
			  {
				  LOG(ERROR) << e.what();
			  }
}



void SipDevice::on_call_invite(eXosip_event_t* event)
{
	LOG(INFO) << "接收到INVITE";
	osip_body_t* sdp_body = nullptr;
	osip_message_get_body(event->request, 0, &sdp_body);
	if (sdp_body != nullptr)
	{
		LOG(INFO) << "request -----> \n" << sdp_body->body;
		LOG(INFO) << "------------------------------------";
	}
	else
	{
		LOG(ERROR) << "SDP Error";
		return;
	}

	sdp_message_t* sdp = NULL;
	if (OSIP_SUCCESS != sdp_message_init(&sdp))
	{
		LOG(ERROR) << "sdp_message_init failed";
		return;
	}

	if (event->request && event->request->req_uri && event->request->req_uri->username)
	{
		_invite_video_channel_id = event->request->req_uri->username;

		LOG(INFO) << "INVITE: " << _invite_video_channel_id;

		_invite_channel_info = nullptr;
		for (auto&& channel : this->Channels)
		{
			if (channel->ID.compare(_invite_video_channel_id) == 0)
			{
				_invite_channel_info = channel;
				break;
			}
		}

		if (_invite_channel_info == nullptr)
		{
			LOG(ERROR) << "未找到对应Channel: " << _invite_video_channel_id;
			return;
		}
	}

	auto ret = sdp_message_parse(sdp, sdp_body->body);
	/*if (OSIP_SUCCESS != ret)
	{
		LOG(ERROR) << "sdp_message_parse failed";
		return;
	}*/

	{
		std::string text = sdp_body->body;
		text = nbase::StringTrim(text);
		auto pos = text.find_last_of('\n');
		if (pos != std::string::npos)
		{
			text = text.substr(pos + 1);
			if (strstr(text.c_str(), "y=0"))
			{
				text = text.substr(2);
				LOG(INFO) << "---Y: " << text;
				_ssrc = text;
			}
		}
	}
	sdp_connection_t* connect = eXosip_get_video_connection(sdp);
	sdp_media_t* media = eXosip_get_video_media(sdp);

	_target_ip = connect->c_addr;
	_target_port = std::stoi(media->m_port);
	auto protocol = media->m_proto;
	_use_tcp = strstr(protocol, "TCP");

	_listen_port = get_port();

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

	auto info = fmt::format(ss.str(), _invite_video_channel_id, this->IP, _local_port, _listen_port, _use_tcp ? "TCP/RTP/AVP" : "RTP/AVP", _ssrc);

	osip_message_t* message = event->request;
	ret = eXosip_call_build_answer(_sip_context, event->tid, 200, &message);
	if (ret != OSIP_SUCCESS)
	{
		LOG(ERROR) << "eXosip_call_build_answer failed";
		return;
	}

	osip_message_set_content_type(message, "APPLICATION/SDP");
	osip_message_set_body(message, info.c_str(), info.length());
	eXosip_call_send_answer(_sip_context, event->tid, 200, message);

	LOG(INFO) << "SDP Response: " << info;
}


void SipDevice::send_response_ok(eXosip_event_t* event) {
	osip_message_t* message = event->request;
	eXosip_message_build_answer(_sip_context, event->tid, 200, &message);

	eXosip_lock(_sip_context);
	eXosip_message_send_answer(_sip_context, event->tid, 200, message);
	eXosip_unlock(_sip_context);
}

void SipDevice::heartbeat_task()
{
	while (_is_running && _register_success && _is_heartbeat_running)
	{
		auto text = R"(<?xml version="1.0"?>
						<Notify>
							<CmdType>Keepalive</CmdType>
							<SN>{}</SN>
							<DeviceID>{}</DeviceID>
							<Status>OK</Status>
						</Notify>
						)"s;
		auto info = fmt::format(text, get_sn(), this->ID);

		osip_message_t* request = nullptr;
		auto ret = eXosip_message_build_request(_sip_context, &request, "MESSAGE", _proxy_uri.c_str(), _from_uri.c_str(), nullptr);
		if (ret != OSIP_SUCCESS)
		{
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


std::string SipDevice::generate_catalog_xml(const std::string& sn)
{
	auto text = R"(<?xml version="1.0" encoding="GB2312"?>
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

	auto device = doc.child("Response").child("DeviceList");
	for (auto&& channel : this->Channels)
	{
		auto node = device.append_child("Item");
		node.append_child("DeviceID").text().set(channel->ID.c_str());
		node.append_child("Name").text().set("Channel");
		node.append_child("Manufacturer").text().set(this->Manufacturer.c_str());
		node.append_child("Model").text().set(this->Name.c_str());
		node.append_child("Status").text().set("ON");
	}

	std::stringstream ss;
	doc.save(ss);
	LOG(INFO) << "Catalog:----------------------------- \n" << ss.str();

	return ss.str();
}
