#include "pch.h"
#include "RequestPool.h"

void RequestPool::Start()
{
	CheckRequestTimeout();
}

void RequestPool::AddRequest(const std::string& req_id, BaseRequest::Ptr request)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _requests.find(req_id);
	if (iter == _requests.end())
	{
		request->SetRequestID(req_id);
		_requests[req_id] = request;
	}
	else
	{
		SPDLOG_INFO("Request Already Exists: {}", req_id);
	}
}

void RequestPool::RemoveRequest(const std::string& req_id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	_requests.erase(req_id);
}

MessageRequest::Ptr RequestPool::GetMessageRequestBySN(const std::string& sn, REQUEST_MESSAGE_TYPE type)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	for (auto&& req : _requests)
	{
		if (req.second->GetRequestType() != type)
		{
			continue;
		}

		MessageRequest::Ptr request = std::dynamic_pointer_cast<MessageRequest>(req.second);
		if (request->GetRequestSN() == sn)
		{
			return request;
		}
	}

	return nullptr;
}

int RequestPool::HandleMessageRequest(const std::string& req_id, int status_code)
{
	return HandleResponse(req_id, status_code);
}

void RequestPool::CheckRequestTimeout(float timeout)
{
	_check_timer.reset(new toolkit::Timer(timeout, [this]()
		{
			//TODO:
			std::scoped_lock<std::mutex> lk(_mutex);
			time_t now = time(nullptr);

			for (auto iter = _requests.begin(); iter != _requests.end(); )
			{
				if (now - iter->second->GetRequestTime() > 6)
				{
					//TODO:
					iter->second->HandleResponse(-1);
					iter->second->Finish();
					iter = _requests.erase(iter);
				}
				else
				{
					++iter;
				}
			}
			return true;
		},
		nullptr)
	);
}

int RequestPool::HandleResponse(const std::string& req_id, int status_code)
{
	BaseRequest::Ptr request = nullptr;
	std::scoped_lock<std::mutex> lk(_mutex);
	auto iter = _requests.find(req_id);
	if (iter != _requests.end())
	{
		auto request = iter->second;
		//if (status_code != SIP_OK)
		{
			toolkit::EventPollerPool::Instance().getExecutor()->async([request, status_code]()
				{
					request->HandleResponse(status_code);
					request->Finish();
				});
		}
	}

	return 0;
}
