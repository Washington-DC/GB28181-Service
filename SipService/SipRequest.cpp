#include "pch.h"
#include "SipRequest.h"
#include "ConfigManager.h"
#include "RequestPool.h"
#include "StreamManager.h"
#include "ZlmServer.h"
#include "PtzCmd.h"
#include "Utils.h"

BaseRequest::BaseRequest(eXosip_t* ctx, Device::Ptr device, REQUEST_MESSAGE_TYPE type)
	:_exosip_context(ctx)
	, _request_type(type)
{
	_device = device;
	_request_time = std::time(nullptr);
}

BaseRequest::~BaseRequest()
{
}

int BaseRequest::HandleResponse(int status_code)
{
	return 0;
}

void BaseRequest::SetWait(bool bwait)
{
	_b_wait = bwait;
}

void BaseRequest::WaitResult()
{
	if (!_b_wait)
		return;
	std::unique_lock<std::mutex> lk(_mutex);
	_cv.wait(lk);
}

bool BaseRequest::IsFinished()
{
	return _b_finished;
}

void BaseRequest::SetRequestID(const std::string& id)
{
	_request_id = id;
}

REQUEST_MESSAGE_TYPE BaseRequest::GetRequestType()
{
	return _request_type;
}

void BaseRequest::OnRequestFinished()
{
	_b_finished = true;
	if (_b_wait)
		Finish();
}

time_t BaseRequest::GetRequestTime()
{
	return _request_time;
}


void BaseRequest::Finish()
{
	if (!_b_wait)
		return;
	std::unique_lock<std::mutex> lk(_mutex);
	lk.unlock();
	_cv.notify_one();
	_b_wait = false;
}


Device::Ptr BaseRequest::GetDevice()
{
	return _device;
}


const char* BaseRequest::_get_request_id_from_request(osip_message_t* msg)
{
	osip_generic_param_t* tag = nullptr;
	//osip_to_get_tag(msg->from, &tag);
	osip_uri_param_get_byname(&msg->from->gen_params, (char*)"tag", &tag);

	if (tag == nullptr || tag->gvalue == nullptr)
	{
		return nullptr;
	}
	return tag->gvalue;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
std::atomic_uint64_t MessageRequest::_sn = 0;

MessageRequest::MessageRequest(eXosip_t* ctx, Device::Ptr device, REQUEST_MESSAGE_TYPE type)
	:BaseRequest(ctx, device, type)
{
	_request_sn = _sn++;
}

MessageRequest::~MessageRequest()
{
}

int MessageRequest::SendMessage(bool needcb)
{
	if (_exosip_context)
	{
		auto config = ConfigManager::GetInstance()->GetSipServerInfo();
		auto from_uri = fmt::format("sip:{}@{}:{}", config->ID, config->IP, config->Port);
		auto to_uri = fmt::format("sip:{}@{}:{}", _device->GetDeviceID(), _device->GetIP(), _device->GetPort());

		osip_message_t* msg = nullptr;
		auto ret = eXosip_message_build_request(_exosip_context, &msg, "MESSAGE", to_uri.c_str(), from_uri.c_str(), nullptr);
		if (ret != OSIP_SUCCESS)
		{
			return ret;
		}

		auto body = make_manscdp_body();
		body = format_xml(body);

		osip_message_set_body(msg, body.c_str(), body.length());
		osip_message_set_content_type(msg, "Application/MANSCDP+xml");

		eXosip_lock(_exosip_context);
		ret = eXosip_message_send_request(_exosip_context, msg);
		eXosip_unlock(_exosip_context);
		//MARK: 这里虽然成功了，但是返回值不是0
		//if (ret != OSIP_SUCCESS)
		//{
		//	return ret;
		//}

		if (needcb)
		{
			std::string request_id = _get_request_id_from_request(msg);
			if (request_id.length() > 0)
			{
				BaseRequest::Ptr request = shared_from_this();
				RequestPool::GetInstance()->AddRequest(request_id, request);
			}
		}
		return 0;
	}

	return -1;
}

const std::string MessageRequest::GetRequestSN()
{
	return std::to_string(_request_sn);
}

const std::string MessageRequest::make_manscdp_body()
{
	return std::string();
}

std::string MessageRequest::format_xml(const std::string& xml)
{
	pugi::xml_document doc;
	auto ret = doc.load_string(xml.c_str());
	if (ret.status != pugi::status_ok) {
		return xml;
	}

	std::stringstream ss;
	doc.save(ss);

	SPDLOG_INFO(ss.str());

	return ss.str();
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const std::string CatalogRequest::make_manscdp_body()
{
	auto text = R"(<?xml version="1.0"?>
								<Query>
								<CmdType>Catalog</CmdType>
								<SN>{}</SN>
								<DeviceID>{}</DeviceID>
								</Query>
								)";

	return fmt::format(text, _request_sn, _device->GetDeviceID());
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const std::string DeviceInfoRequest::make_manscdp_body()
{
	auto text = R"(<?xml version="1.0"?>
								<Query>
								<CmdType>DeviceInfo</CmdType>
								<SN>{}</SN>
								<DeviceID>{}</DeviceID>
								</Query>
								)";

	return fmt::format(text, _request_sn, _device->GetDeviceID());
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const std::string InviteRequest::make_sdp_body(const std::string& id, int port, const std::string& ssrc)
{
	//测试，只有将t放在前边才能够相机被识别到，不然会返回 415 Unsupported Media Type

	/*
		根据SDP（Session Description Protocol）的规范，SDP 中的各个字段其实是没有顺序要求的。
		每一行的含义都是独立的，可以按任意顺序排列。
		但是，为了方便阅读和解析，通常会按照规定的顺序来排列。
			v= (协议版本)
			o= (创建者和会话标识符)
			s= (会话名称)
			i=* (会话信息)
			u=* (描述的 URI)
			e=* (电子邮件地址)
			p=* (电话号码)
			c=* (连接信息 - 如果在所有媒体中都包含则不需要)
			b=* (带宽信息)
			t=* (会话起止时间)
			r=* (重复时间)
			z=* (时区调整)
			k=* (加密密钥)
			a=* (零个或多个会话属性行)
			Zero or more media descriptions
	*/

	if (_play_mode == SSRCConfig::Mode::Playback)
	{
		std::stringstream ss;

		ss << "v=0\r\n";
		ss << "o={} 0 0 IN IP4 {}\r\n";
		ss << "s={}\r\n";
		ss << "u={}:0\r\n";//u字段顺序错误，会导致海康NVR回复400错误
		ss << "c=IN IP4 {}\r\n";
		ss << "t={} {}\r\n";
		ss << "m=video {} RTP/AVP 96 97 98 99\r\n";
		ss << "a=recvonly\r\n";
		ss << "a=rtpmap:96 PS/90000\r\n";
		ss << "a=rtpmap:97 MPEG4/90000\r\n";
		ss << "a=rtpmap:98 H264/90000\r\n";
		ss << "a=rtpmap:99 H265/90000\r\n";
		ss << "y={}\r\n";

		auto server = ConfigManager::GetInstance()->GetSipServerInfo();
		return fmt::format(ss.str(), id, _device->GetStreamIP(), "Playback", id,
			_device->GetStreamIP(), _start_time, _end_time,  port, ssrc);
	}
	else
	{
		auto text = R"(v=0
o={} 0 0 IN IP4 {}
s={}
c=IN IP4 {}
t={} {}
m=video {} RTP/AVP 96 97 98 99
a=recvonly
a=rtpmap:96 PS/90000
a=rtpmap:97 MPEG4/90000
a=rtpmap:98 H264/90000
a=rtpmap:99 H265/90000
y={}
)";
		auto server = ConfigManager::GetInstance()->GetSipServerInfo();
		return fmt::format(text, id, _device->GetStreamIP(), "Play",
			_device->GetStreamIP(), _start_time, _end_time, port, ssrc);
	}
}


int InviteRequest::SendCall(bool needcb)
{
	auto config = ConfigManager::GetInstance()->GetSipServerInfo();
	auto from_uri = fmt::format("sip:{}@{}:{}", config->ID, config->IP, config->Port);
	auto to_uri = fmt::format("sip:{}@{}:{}", _channel_id, _device->GetIP(), _device->GetPort());

	osip_message_t* msg = nullptr;
	auto channel = _device->GetChannel(_channel_id);

	auto subject = fmt::format("{}:{},{}:0", _channel_id, _ssrc, config->ID);
	SPDLOG_INFO("subject: {}", subject);

	eXosip_lock(_exosip_context);
	auto ret = eXosip_call_build_initial_invite(_exosip_context, &msg, to_uri.c_str(), from_uri.c_str(), nullptr, subject.c_str());
	eXosip_unlock(_exosip_context);

	if (ret != OSIP_SUCCESS)
	{
		SPDLOG_ERROR("eXosip_call_build_initial_invite error: {}", ret);
		return -1;
	}

	//单端口模式时，使用ZLM的固定端口
	//多端口模式时，使用OpenRtpServer创建RTP接收端口
	int rtp_port = -1;
	if (ZlmServer::GetInstance()->SinglePortMode())
	{
		rtp_port = ZlmServer::GetInstance()->FixedRtpPort();
	}
	else
	{
		rtp_port = ZlmServer::GetInstance()->OpenRtpServer(_stream_id);
	}

	if (rtp_port == -1)
	{
		SPDLOG_ERROR("创建RTP服务器失败");
		return -1;
	}

	SPDLOG_INFO("Invite: {}", _stream_id);

	_ssrc_info = std::make_shared<SSRCInfo>(rtp_port, _ssrc, _stream_id);
	auto session = std::make_shared<CallSession>("rtp", _stream_id, _ssrc_info);
	StreamManager::GetInstance()->AddStream(session);

	auto sdp_body = make_sdp_body(_channel_id, rtp_port, _ssrc);

	osip_message_set_body(msg, sdp_body.c_str(), sdp_body.length());
	osip_message_set_content_type(msg, "APPLICATION/SDP");
	//std::string session_expires = "1800;refresher=uac";
	//osip_message_set_header(msg, "session-expires", session_expires.c_str());
	//osip_message_set_supported(msg, "timer");
	eXosip_lock(_exosip_context);
	int call_id = eXosip_call_send_initial_invite(_exosip_context, msg);
	eXosip_unlock(_exosip_context);

	if (call_id > 0)
	{
		SPDLOG_INFO("eXosip_call_send_initial_invite: {}", call_id);
	}
	session->SetCallID(call_id);
	session->exosip_context = _exosip_context;
	SPDLOG_INFO("==================================SDP: \n {}", sdp_body);

	if (needcb)
	{
		std::string request_id = _get_request_id_from_request(msg);
		if (!request_id.empty())
		{
			BaseRequest::Ptr request = shared_from_this();
			RequestPool::GetInstance()->AddRequest(request_id, request);
		}
	}
	return 0;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

const std::string PresetRequest::make_manscdp_body()
{
	auto text = R"(<?xml version="1.0"?>
					<Query>
						<CmdType>PresetQuery</CmdType>
						<SN>{}</SN>
						<DeviceID>{}</DeviceID>
					</Query>
					)";

	return fmt::format(text, _request_sn, _channel_id);
}

void PresetRequest::InsertPreset(const std::string& preset_id, const std::string& preset_name)
{
	_presets.push_back({ preset_id ,preset_name });
}

const std::vector<std::pair<std::string, std::string>> PresetRequest::GetPresetList()
{
	return _presets;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const std::string PresetCtlRequest::make_manscdp_body()
{
	auto text = R"(<?xml version="1.0"?>
					<Control>
						<CmdType>DeviceControl</CmdType>
						<SN>{}</SN>
						<DeviceID>{}</DeviceID>
						<PTZCmd>{}</PTZCmd>
						<Info>
							<ControlPriority>5</ControlPriority>
						</Info>
					</Control>
					)";

	return fmt::format(text, _request_sn, _channel_id, PtzCmd::cmdCode(_byte4, _byte5, _byte6, _byte7));
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

const std::string PtzCtlRequest::make_manscdp_body()
{
	auto text = R"(<?xml version="1.0"?>
					<Control>
						<CmdType>DeviceControl</CmdType>
						<SN>{}</SN>
						<DeviceID>{}</DeviceID>
						<PTZCmd>{}</PTZCmd>
						<Info>
							<ControlPriority>5</ControlPriority>
						</Info>
					</Control>
					)";

	return fmt::format(text, _request_sn, _channel_id, PtzCmd::cmdString(_leftRight, _upDown, _inOut, _moveSpeed, _zoomSpeed));

}

int PtzCtlRequest::HandleResponse(int statcode)
{
	_leftRight = 0;
	_upDown = 0;
	_inOut = 0;
	_moveSpeed = 0;
	_zoomSpeed = 0;

	// 收到相机回复后，立即停止云台转动
	SPDLOG_INFO("PtzControlRequest HandleResponse statuscode = {}", statcode);
	//SendMessage(false);
	return 0;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


const std::string LensCtlRequest::make_manscdp_body()
{
	auto text = R"(<?xml version="1.0"?>
					<Control>
						<CmdType>DeviceControl</CmdType>
						<SN>{}</SN>
						<DeviceID>{}</DeviceID>
						<PTZCmd>{}</PTZCmd>
						<Info>
							<ControlPriority>5</ControlPriority>
						</Info>
					</Control>
					)";

	return fmt::format(text, _request_sn, _channel_id, PtzCmd::cmdLens(_iris, _focus, _iris_speed, _focus_speed));
}

int LensCtlRequest::HandleResponse(int statcode)
{
	return 0;
}



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------



const std::string RecordRequest::make_manscdp_body()
{
	auto text = R"(<?xml version="1.0"?>
					<Query>
						<CmdType>RecordInfo</CmdType>
						<SN>{}</SN>
						<DeviceID>{}</DeviceID>
						<StartTime>{}</StartTime>
						<EndTime>{}</EndTime>
						<Secrecy>0</Secrecy>
						<Type>all</Type>
					</Query>
					)";

	return fmt::format(text, _request_sn, _channel_id,
		toolkit::getTimeStr("%Y-%m-%dT%H:%M:%S", _start_time),
		toolkit::getTimeStr("%Y-%m-%dT%H:%M:%S", _end_time));
}

void RecordRequest::InsertRecord(std::shared_ptr<RecordItem> item)
{
	_record_items.push_back(item);
}

uint32_t RecordRequest::GetRecordSize()
{
	return (uint32_t)_record_items.size();
}

std::vector<std::shared_ptr<RecordItem>> RecordRequest::GetRecordList()
{
	return _record_items;
}

int CallMessageRequest::SetSpeed(float speed)
{
	osip_message_t* msg = nullptr;
	auto ret = eXosip_call_build_info(_exosip_context, _session->GetDialogID(), &msg);
	if (ret != OSIP_SUCCESS)
	{
		return ret;
	}

	auto text = "PAUSE RTSP/1.0\r\nCSeq: {}\r\nScale: {}\r\n";
	auto body = fmt::format(text, _session->GetCSeqID(), speed);

	osip_message_set_body(msg, body.c_str(), body.length());
	osip_message_set_content_type(msg, "Application/MANSCDP");

	eXosip_lock(_exosip_context);
	ret = eXosip_call_send_request(_exosip_context, _session->GetDialogID(), msg);
	eXosip_unlock(_exosip_context);

	return 0;
}

int CallMessageRequest::SetPause(bool flag)
{
	osip_message_t* msg = nullptr;
	auto ret = eXosip_call_build_info(_exosip_context, _session->GetDialogID(), &msg);
	if (ret != OSIP_SUCCESS)
	{
		return ret;
	}
	// 暂停播放
	// 这里也有可能是MANSRTSP/1.0, 暂按照文档来实现
	auto text = "PAUSE RTSP/1.0\r\nCSeq: {}\r\nPauseTime: now\r\n";
	if (!flag)
	{	
		// 继续播放
		text = "PLAY RTSP/1.0\r\nCSeq: {}\r\nRange: npt=now-\r\n";
	}
	auto body = fmt::format(text, _session->GetCSeqID());

	osip_message_set_body(msg, body.c_str(), body.length());
	osip_message_set_content_type(msg, "Application/MANSCDP");

	eXosip_lock(_exosip_context);
	ret = eXosip_call_send_request(_exosip_context, _session->GetDialogID(), msg);
	eXosip_unlock(_exosip_context);
	return 0;
}

int CallMessageRequest::SetRange(int64_t timestamp)
{
	osip_message_t* msg = nullptr;
	auto ret = eXosip_call_build_info(_exosip_context, _session->GetDialogID(), &msg);
	if (ret != OSIP_SUCCESS)
	{
		return ret;
	}

	auto text = "PLAY RTSP/1.0\r\nCSeq: {}\r\nRange: npt={}-\r\n";
	auto body = fmt::format(text, _session->GetCSeqID(), timestamp);

	osip_message_set_body(msg, body.c_str(), body.length());
	osip_message_set_content_type(msg, "Application/MANSCDP");

	eXosip_lock(_exosip_context);
	ret = eXosip_call_send_request(_exosip_context, _session->GetDialogID(), msg);
	eXosip_unlock(_exosip_context);
	return 0;
}
