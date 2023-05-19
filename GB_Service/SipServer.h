#pragma once
#include "EventHandlerManager.h"
class SipServer
{
public:
	SINGLETON_DEFINE(SipServer);

	bool Init(const std::string& sip_id = "34020000002000000001", uint16_t port = 5060);
	bool Start();
	bool Stop();

	eXosip_t* GetSipContext();

private:
	SipServer() = default;
	void RecvEventThread();

	SipEvent::Ptr _new_event(eXosip_t* exosip_context, eXosip_event_t* exosip_event);


private:
	bool _start = false;
	eXosip_t* _sip_context = nullptr;
	std::string _sip_id = "34020000002000000001";
	int _sip_port = 5060;
	std::string _user_agent = "cdtye_gb28181";
	uint64_t _event_id = 0;      // 事件id 自增
	std::shared_ptr<std::thread> _event_thread = nullptr;

	EventHandlerManager _event_handler_manager;
};

