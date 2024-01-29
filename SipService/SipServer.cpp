#include "pch.h"
#include "SipServer.h"

bool SipServer::Init(const std::string& sip_id, uint16_t port, bool use_tcp)
{
	_sip_id = sip_id;
	_sip_port = port;
	_listen_tcp = use_tcp;

	_sip_context = eXosip_malloc();
	if (OSIP_SUCCESS != eXosip_init(_sip_context)) {
		LOG(ERROR) << "eXosip_init failed";
		return false;
	}

	eXosip_set_user_agent(_sip_context, _user_agent.c_str());
	int val = 1;
	eXosip_set_option(_sip_context, EXOSIP_OPT_ENABLE_REUSE_TCP_PORT, (void*)&val);

	return true;
}

bool SipServer::Start()
{
	auto ret = eXosip_listen_addr(_sip_context, _listen_tcp ? IPPROTO_TCP : IPPROTO_UDP, NULL, _sip_port, AF_INET, 0);
	if (ret != OSIP_SUCCESS)
	{
		LOG(ERROR) << "eXosip_listen_addr";
		eXosip_quit(_sip_context);
		return false;
	}

	LOG(INFO) << "Start SipServer...";
	_start = true;
	_event_thread = std::make_shared<std::thread>(&SipServer::RecvEventThread, this);

	return true;
}

bool SipServer::Stop()
{
	_start = false;

	if (_event_thread && _event_thread->joinable())
	{
		_event_thread->join();
		_event_thread = nullptr;
	}
	LOG(INFO) << "Stop SipServer...";

	eXosip_quit(_sip_context);
	_sip_context = nullptr;
	return true;
}


eXosip_t* SipServer::GetSipContext()
{
	return _sip_context;
}


void SipServer::RecvEventThread()
{
	while (_start)
	{
		eXosip_event_t* event = nullptr;
		event = eXosip_event_wait(_sip_context, 0, 20);

		if (event == nullptr)
		{
			continue;
		}

		{
			eXosip_lock(_sip_context);
			eXosip_automatic_action(_sip_context);
			eXosip_unlock(_sip_context);
		}

		auto sip_event = _new_event(_sip_context, event);
		if (sip_event == nullptr)
		{
			continue;
		}

		LOG(INFO) << "Receive Event: " << sip_event->name;

		toolkit::EventPollerPool::Instance().getExecutor()->async([sip_event]()
			{
				sip_event->proc(sip_event);
				eXosip_event_free(sip_event->exosip_event);
			});
	}
}

SipEvent::Ptr SipServer::_new_event(eXosip_t* exosip_context, eXosip_event_t* exosip_event)
{
	if (exosip_event == nullptr)
		return nullptr;

	if (exosip_event->type < EXOSIP_REGISTRATION_SUCCESS ||
		exosip_event->type > EXOSIP_NOTIFICATION_GLOBALFAILURE)
		return nullptr;

	SipEvent::Ptr event = std::make_shared<SipEvent>();
	EventHandlerManager::EventNameProcPair pair = _event_handler_manager.GetEventProc(exosip_event->type);
	if (pair.name == nullptr)
		return nullptr;

	event->name = pair.name;
	event->proc = pair.proc;
	event->exosip_context = exosip_context;
	event->exosip_event = exosip_event;
	event->id = _event_id++;

	return event;
}
