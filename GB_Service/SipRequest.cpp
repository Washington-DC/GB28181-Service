#include "pch.h"
#include "SipRequest.h"
#include "ConfigManager.h"
#include "RequestPool.h"
#include "StreamManager.h"

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

int BaseRequest::HadnleResponse(int status_code)
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
	osip_to_get_tag(msg->from, &tag);
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



const std::string InviteRequest::make_sdp_body()
{
	auto text = R"(v=0
o={} 0 0 IN IP4 {}
s=Play
c=IN IP4 {}
t=0 0
m=video {} RTP/AVP 96 98 97
a=recvonly
a=rtpmap:96 PS/90000
a=rtpmap:98 H264/90000
a=rtpmap:97 MPEG4/90000
a=setup:passive
a=connection:new
y={}
f=)";

	auto server = ConfigManager::GetInstance()->GetSipServerInfo();
	return fmt::format(text, server->ID, server->IP, server->IP, _ssrc->GetPort(), _ssrc->GetSSRC());
}


int InviteRequest::SendCall(bool needcb)
{
	auto config = ConfigManager::GetInstance()->GetSipServerInfo();
	auto from_uri = fmt::format("sip:{}@{}:{}", config->ID, config->IP, config->Port);
	auto to_uri = fmt::format("sip:{}@{}:{}", _device->GetDeviceID(), _device->GetIP(), _device->GetPort());

	osip_message_t* msg = nullptr;
	eXosip_lock(_exosip_context);
	auto ret = eXosip_call_build_initial_invite(_exosip_context, &msg, to_uri.c_str(), from_uri.c_str(), nullptr, nullptr);
	eXosip_unlock(_exosip_context);

	if (ret != OSIP_SUCCESS)
	{
		LOG(ERROR) << "eXosip_call_build_initial_invite error: " << ret;
		return -1;
	}

	auto stream_id = fmt::format("{}_{}", _device->GetDeviceID(), _channel_id);

	//TODO:
	_ssrc = std::make_shared<SSRCInfo>(23045, "0123456789", stream_id);

	auto session = std::make_shared<CallSession>("rtp", stream_id, _ssrc);
	StreamManager::GetInstance()->AddStream(session);

	auto sdp_body = make_sdp_body();

	osip_message_set_body(msg, sdp_body.c_str(), sdp_body.length());
	osip_message_set_content_type(msg, "application/sdp");
	std::string session_expires = "1800;refresher=uac";
	osip_message_set_header(msg, "session-expires", session_expires.c_str());
	osip_message_set_supported(msg, "timer");
	eXosip_lock(_exosip_context);
	int call_id = eXosip_call_send_initial_invite(_exosip_context, msg);
	eXosip_unlock(_exosip_context);

	if (call_id > 0)
	{
		LOG(INFO) << "eXosip_call_send_initial_invite: " << call_id;
	}
	session->SetCallID(call_id);
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


