#pragma once

#include "Structs.h"

class BaseRequest
{
public:
	typedef std::shared_ptr<BaseRequest> Ptr;
	explicit BaseRequest(REQUEST_MESSAGE_TYPE type);
	virtual ~BaseRequest();

	virtual int HadnleResponse(int status_code);
	void SetWait(bool bwait = true);
	void WaitResult();
	bool IsFinished();
	int SetRequestID(std::string& id);

	REQUEST_MESSAGE_TYPE GetRequestType();

	int OnRequestFinished();


public:
	void _finished();
	const char* _get_request_id_from_request(osip_message_t* msg);

protected:
	std::string _request_id;

private:
	bool _b_finished = false;
	bool _b_wait = false;
	REQUEST_MESSAGE_TYPE _request_type;

	std::mutex _mutex;
	std::condition_variable _cv;
};

