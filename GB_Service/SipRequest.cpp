#include "pch.h"
#include "SipRequest.h"

BaseRequest::BaseRequest(REQUEST_MESSAGE_TYPE type)
{
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

int BaseRequest::SetRequestID(std::string& id)
{
	_request_id = id;
	return 0;
}

REQUEST_MESSAGE_TYPE BaseRequest::GetRequestType()
{
	return _request_type;
}

int BaseRequest::OnRequestFinished()
{
	_b_finished = true;
	if (_b_wait)
		_finished();
	return 0;
}

void BaseRequest::_finished()
{
	if (!_b_wait)
		return;
	std::unique_lock<std::mutex> lk(_mutex);
	lk.unlock();
	_cv.notify_one();
	_b_wait = false;
}

const char* BaseRequest::_get_request_id_from_request(osip_message_t* msg)
{
	osip_generic_param_t* tag = nullptr;
	osip_to_get_tag(msg->from,&tag);
	if (tag == nullptr || tag->gvalue == nullptr)
	{
		return nullptr;
	}
	return tag->gvalue;
}
