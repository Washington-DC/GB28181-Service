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
	_request_time = time(nullptr);
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
const std::string InviteRequest::make_sdp_body(const std::string& id)
{
	auto text = R"(v=0
o={} 0 0 IN IP4 {}
s=Play
c=IN IP4 {}
t=0 0
m=video {} RTP/AVP 96 97 98 99
a=recvonly
a=rtpmap:96 PS/90000
a=rtpmap:97 MPEG4/90000
a=rtpmap:98 H264/90000
a=rtpmap:99 H265/90000
y={}
)";

	auto server = ConfigManager::GetInstance()->GetSipServerInfo();
	return fmt::format(text, id, _device->GetStreamIP(), _device->GetStreamIP(), _ssrc_info->GetPort(), _ssrc_info->GetSSRC());
}


int InviteRequest::SendCall(bool needcb)
{
	auto config = ConfigManager::GetInstance()->GetSipServerInfo();
	auto from_uri = fmt::format("sip:{}@{}:{}", config->ID, config->IP, config->Port);
	auto to_uri = fmt::format("sip:{}@{}:{}", _channel_id, _device->GetIP(), _device->GetPort());

	osip_message_t* msg = nullptr;
	auto channel = _device->GetChannel(_channel_id);
	std::string ssrc = "";
	if (ZlmServer::GetInstance()->SinglePortMode())
	{
		ssrc = channel->GetDefaultSSRC();
	}
	else
	{
		ssrc = SSRCConfig::GetInstance()->GenerateSSRC();
	}

	auto subject = fmt::format("{}:{},{}:0", _channel_id, ssrc, config->ID);

	LOG(INFO) << "subject: " << subject;

	eXosip_lock(_exosip_context);
	auto ret = eXosip_call_build_initial_invite(_exosip_context, &msg, to_uri.c_str(), from_uri.c_str(), nullptr, subject.c_str());
	eXosip_unlock(_exosip_context);

	if (ret != OSIP_SUCCESS)
	{
		LOG(ERROR) << "eXosip_call_build_initial_invite error: " << ret;
		return -1;
	}

	std::string stream_id = "";

	//单端口模式时，使用ZLM的固定端口
	//多端口模式时，使用OpenRtpServer创建RTP接收端口
	int rtp_port = -1;
	if (ZlmServer::GetInstance()->SinglePortMode())
	{
		stream_id = SSRC_Hex(ssrc);
		rtp_port = ZlmServer::GetInstance()->FixedRtpPort();
	}
	else
	{
		stream_id = fmt::format("{}_{}", _device->GetDeviceID(), _channel_id);
		rtp_port = ZlmServer::GetInstance()->OpenRtpServer(stream_id);
	}

	if (rtp_port == -1)
	{
		LOG(ERROR) << "创建RTP服务器失败";
		return -1;
	}

	LOG(INFO) << "Invite: " << stream_id;

	_ssrc_info = std::make_shared<SSRCInfo>(rtp_port, ssrc, stream_id);
	auto session = std::make_shared<CallSession>("rtp", stream_id, _ssrc_info);
	StreamManager::GetInstance()->AddStream(session);

	auto sdp_body = make_sdp_body(_channel_id);

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
		LOG(INFO) << "eXosip_call_send_initial_invite: " << call_id;
	}
	session->SetCallID(call_id);
	session->exosip_context = _exosip_context;
	LOG(INFO) << "==================================SDP: \n" << sdp_body;

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

	LOG(INFO) << "PtzControlRequest HandleResponse statuscode = " << statcode;
	//SendMessage(false);
	return 0;
}

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
