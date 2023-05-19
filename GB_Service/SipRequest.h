#pragma once
#include "Structs.h"
#include "Device.h"
#include "SSRC_Config.h"
class BaseRequest :public std::enable_shared_from_this<BaseRequest>
{
public:
	typedef std::shared_ptr<BaseRequest> Ptr;
	BaseRequest(eXosip_t* ctx, Device::Ptr device, REQUEST_MESSAGE_TYPE type);
	virtual ~BaseRequest();

	virtual int HadnleResponse(int status_code);
	void SetWait(bool bwait = true);
	void WaitResult();
	void Finish();
	bool IsFinished();
	void SetRequestID(const std::string& id);
	void OnRequestFinished();
	time_t GetRequestTime();
	REQUEST_MESSAGE_TYPE GetRequestType();


protected:
	const char* _get_request_id_from_request(osip_message_t* msg);
	Device::Ptr GetDevice();

protected:
	std::string _request_id;
	eXosip_t* _exosip_context;
	Device::Ptr _device = nullptr;
private:
	bool _b_finished = false;
	bool _b_wait = false;
	REQUEST_MESSAGE_TYPE _request_type = REQUEST_TYPE_UNKNOWN;
	time_t _request_time;

	std::mutex _mutex;
	std::condition_variable _cv;
};


class MessageRequest : public BaseRequest
{
public:
	typedef std::shared_ptr<MessageRequest> Ptr;
	MessageRequest(eXosip_t* ctx, Device::Ptr device, REQUEST_MESSAGE_TYPE type);
	virtual ~MessageRequest();
	int SendMessage(bool needcb = true);
	const std::string GetRequestSN();

protected:
	virtual const std::string make_manscdp_body() = 0;

protected:
	uint64_t _request_sn;

private:
	static std::atomic_uint64_t _sn;
};


class CatalogRequest :public MessageRequest
{
public:
	typedef std::shared_ptr<CatalogRequest> Ptr;
	CatalogRequest(eXosip_t* ctx, Device::Ptr device)
		:MessageRequest(ctx, device, REQUEST_MESSAGE_TYPE::QUERY_CATALOG)
	{

	}

public:
	virtual const std::string make_manscdp_body();

};


class DeviceInfoRequest :public MessageRequest
{
public:
	typedef std::shared_ptr<DeviceInfoRequest> Ptr;
	DeviceInfoRequest(eXosip_t* ctx, Device::Ptr device)
		:MessageRequest(ctx, device, REQUEST_MESSAGE_TYPE::QUERY_DEVICEINFO)
	{

	}

public:
	virtual const std::string make_manscdp_body();

};



class InviteRequest :public BaseRequest
{
public:
	typedef std::shared_ptr<InviteRequest> Ptr;
	InviteRequest(eXosip_t* ctx, Device::Ptr device, const std::string& channel_id)
		:BaseRequest(ctx, device, REQUEST_MESSAGE_TYPE::REQUEST_CALL_INVITE)
		, _channel_id(channel_id)
	{
	};

	virtual int SendCall(bool needcb = true);

protected:
	virtual const std::string make_sdp_body();

private:
	SSRCInfo::Ptr _ssrc;
	std::string _channel_id;
};