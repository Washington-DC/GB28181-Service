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

	virtual int HandleResponse(int status_code);
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

	std::string format_xml(const std::string& xml);

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
	SSRCInfo::Ptr _ssrc_info = nullptr;
	std::string _channel_id;
};



class PresetRequest :public MessageRequest
{
public:
	typedef std::shared_ptr<PresetRequest> Ptr;

	PresetRequest(eXosip_t* ctx, Device::Ptr device, const std::string& channel_id)
		:MessageRequest(ctx, device, REQUEST_MESSAGE_TYPE::DEVICE_QUERY_PRESET)
		, _channel_id(channel_id)
	{

	};
	virtual const std::string make_manscdp_body();

	void InsertPreset(const std::string& preset_id, const std::string& preset_name);

	const std::vector<std::pair<std::string, std::string>> GetPresetList();

private:
	std::string _channel_id;

	std::vector<std::pair<std::string, std::string>> _presets;
};




class PresetCtlRequest :public MessageRequest
{
public:
	typedef std::shared_ptr<PresetCtlRequest> Ptr;

	PresetCtlRequest(eXosip_t* ctx, Device::Ptr device, const std::string& channel_id, int byte4, int byte5, int byte6, int byte7)
		:MessageRequest(ctx, device, REQUEST_MESSAGE_TYPE::DEVICE_CONTROL_PRESET)
		, _channel_id(channel_id)
		, _byte4(byte4)
		, _byte5(byte5)
		, _byte6(byte6)
		, _byte7(byte7)
	{

	};
	virtual const std::string make_manscdp_body();

private:
	std::string _channel_id;
	int _byte4 = 0;
	int _byte5 = 0;
	int _byte6 = 0;
	int _byte7 = 0;
};


class PtzCtlRequest :public MessageRequest
{
public:
	typedef std::shared_ptr<PtzCtlRequest> Ptr;

	PtzCtlRequest(eXosip_t* ctx, Device::Ptr device,
		const std::string& channelId,
		int                leftRight,
		int                upDown,
		int                inOut,
		int                moveSpeed,
		int                zoomSpeed)
		: MessageRequest(ctx, device, REQUEST_MESSAGE_TYPE::DEVICE_CONTROL_PTZ),
		_leftRight(leftRight),
		_upDown(upDown),
		_inOut(inOut),
		_moveSpeed(moveSpeed),
		_zoomSpeed(zoomSpeed),
		_channel_id(channelId) {}

	virtual const std::string make_manscdp_body() override;

	virtual int HandleResponse(int statcode) override;

private:
	int _leftRight = 0;
	int _upDown = 0;
	int _inOut = 0;
	int _moveSpeed = 0;
	int _zoomSpeed = 0;
	std::string _channel_id;
};



class LensCtlRequest :public MessageRequest
{
public:
	typedef std::shared_ptr<LensCtlRequest> Ptr;

	LensCtlRequest(eXosip_t* ctx, Device::Ptr device,
		const std::string& channelId,
		int                iris,
		int                focus,
		int                iris_speed,
		int                focus_speed)
		: MessageRequest(ctx, device, REQUEST_MESSAGE_TYPE::DEVICE_CONTROL_PTZ),
		_iris(iris),
		_focus(focus),
		_iris_speed(iris_speed),
		_focus_speed(focus_speed),
		_channel_id(channelId) {}

	virtual const std::string make_manscdp_body() override;

	virtual int HandleResponse(int statcode) override;

private:
	int _iris = 0;
	int _focus = 0;
	int _iris_speed = 0;
	int _focus_speed = 0;
	std::string _channel_id;
};