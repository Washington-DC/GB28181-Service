#pragma once
#include "SipRequest.h"

class RequestPool
{
public:
	SINGLETON_DEFINE(RequestPool);

	void Start();
	void AddRequest(const std::string& req_id, BaseRequest::Ptr request);
	void RemoveRequest(const std::string& req_id);
	MessageRequest::Ptr GetMessageRequestBySN(const std::string& sn,REQUEST_MESSAGE_TYPE type);
	int HandleMessageRequest(const std::string& req_id,int status_code);

private:
	RequestPool() = default;
	void CheckRequestTimeout(float timeout = 6.0);
	int HandleResponse(const std::string& req_id, int status_code);

private:
	std::map<std::string, BaseRequest::Ptr> _requests;
	toolkit::Timer::Ptr _check_timer = nullptr;
	std::mutex _mutex;
};

