#include "pch.h"
#include "EventHandlerManager.h"

#define CALLBACK_TEMPLATE(F) (std::bind(&EventHandlerManager::F, this, std::placeholders::_1))


EventHandlerManager::EventHandlerManager()
{
	EventNameProcPair eventProcTable[EXOSIP_EVENT_COUNT] = {
		   {"EXOSIP_REGISTRATION_SUCCESS",        CALLBACK_TEMPLATE(on_exosip_registration_success)},
		   {"EXOSIP_REGISTRATION_FAILURE",        CALLBACK_TEMPLATE(on_exosip_registration_failure)},
		   {"EXOSIP_CALL_INVITE",                 CALLBACK_TEMPLATE(on_exosip_call_invite)},
		   {"EXOSIP_CALL_REINVITE",               CALLBACK_TEMPLATE(on_exosip_call_reinvite)},
		   {"EXOSIP_CALL_NOANSWER",               CALLBACK_TEMPLATE(on_exosip_call_noanswer)},
		   {"EXOSIP_CALL_PROCEEDING",             CALLBACK_TEMPLATE(on_exosip_call_proceeding)},
		   {"EXOSIP_CALL_RINGING",                CALLBACK_TEMPLATE(on_exosip_call_ringing)},
		   {"EXOSIP_CALL_ANSWERED",               CALLBACK_TEMPLATE(on_exosip_call_answered)},
		   {"EXOSIP_CALL_REDIRECTED",             CALLBACK_TEMPLATE(on_exosip_call_redirected)},
		   {"EXOSIP_CALL_REQUESTFAILURE",         CALLBACK_TEMPLATE(on_exosip_call_requestfailure)},
		   {"EXOSIP_CALL_SERVERFAILURE",          CALLBACK_TEMPLATE(on_exosip_call_serverfailure)},
		   {"EXOSIP_CALL_GLOBALFAILURE",          CALLBACK_TEMPLATE(on_exosip_call_globalfailure)},
		   {"EXOSIP_CALL_ACK",                    CALLBACK_TEMPLATE(on_exosip_call_ack)},
		   {"EXOSIP_CALL_CANCELLED",              CALLBACK_TEMPLATE(on_exosip_call_cancelled)},
		   {"EXOSIP_CALL_MESSAGE_NEW",            CALLBACK_TEMPLATE(on_exosip_call_message_new)},
		   {"EXOSIP_CALL_MESSAGE_PROCEEDING",     CALLBACK_TEMPLATE(on_exosip_call_message_proceeding)},
		   {"EXOSIP_CALL_MESSAGE_ANSWERED",       CALLBACK_TEMPLATE(on_exosip_call_message_answered)},
		   {"EXOSIP_CALL_MESSAGE_REDIRECTED",     CALLBACK_TEMPLATE(on_exosip_call_message_redirected)},
		   {"EXOSIP_CALL_MESSAGE_REQUESTFAILURE", CALLBACK_TEMPLATE(on_exosip_call_message_requestfailure)},
		   {"EXOSIP_CALL_MESSAGE_SERVERFAILURE",  CALLBACK_TEMPLATE(on_exosip_call_message_serverfailure)},
		   {"EXOSIP_CALL_MESSAGE_GLOBALFAILURE",  CALLBACK_TEMPLATE(on_exosip_call_message_globalfailure)},
		   {"EXOSIP_CALL_CLOSED",                 CALLBACK_TEMPLATE(on_exosip_call_closed)},
		   {"EXOSIP_CALL_RELEASED",               CALLBACK_TEMPLATE(on_exosip_call_released)},
		   {"EXOSIP_MESSAGE_NEW",                 CALLBACK_TEMPLATE(on_exosip_message_new)},
		   {"EXOSIP_MESSAGE_PROCEEDING",          CALLBACK_TEMPLATE(on_exosip_message_proceeding)},
		   {"EXOSIP_MESSAGE_ANSWERED",            CALLBACK_TEMPLATE(on_exosip_message_answered)},
		   {"EXOSIP_MESSAGE_REDIRECTED",          CALLBACK_TEMPLATE(on_exosip_message_redirected)},
		   {"EXOSIP_MESSAGE_REQUESTFAILURE",      CALLBACK_TEMPLATE(on_exosip_message_requestfailure)},
		   {"EXOSIP_MESSAGE_SERVERFAILURE",       CALLBACK_TEMPLATE(on_exosip_message_serverfailure)},
		   {"EXOSIP_MESSAGE_GLOBALFAILURE",       CALLBACK_TEMPLATE(on_exosip_message_globalfailure)},
		   {"EXOSIP_SUBSCRIPTION_NOANSWER",       CALLBACK_TEMPLATE(on_exosip_subscription_noanswer)},
		   {"EXOSIP_SUBSCRIPTION_PROCEEDING",     CALLBACK_TEMPLATE(on_exosip_subscription_proceeding)},
		   {"EXOSIP_SUBSCRIPTION_ANSWERED",       CALLBACK_TEMPLATE(on_exosip_subscription_answered)},
		   {"EXOSIP_SUBSCRIPTION_REDIRECTED",     CALLBACK_TEMPLATE(on_exosip_subscription_redirected)},
		   {"EXOSIP_SUBSCRIPTION_REQUESTFAILURE", CALLBACK_TEMPLATE(on_exosip_subscription_requestfailure)},
		   {"EXOSIP_SUBSCRIPTION_SERVERFAILURE",  CALLBACK_TEMPLATE(on_exosip_subscription_serverfailure)},
		   {"EXOSIP_SUBSCRIPTION_GLOBALFAILURE",  CALLBACK_TEMPLATE(on_exosip_subscription_globalfailure)},
		   {"EXOSIP_SUBSCRIPTION_NOTIFY",         CALLBACK_TEMPLATE(on_exosip_subscription_notify)},
		   {"EXOSIP_IN_SUBSCRIPTION_NEW",         CALLBACK_TEMPLATE(on_exosip_in_subscription_new)},
		   {"EXOSIP_NOTIFICATION_NOANSWER",       CALLBACK_TEMPLATE(on_exosip_notification_noanswer)},
		   {"EXOSIP_NOTIFICATION_PROCEEDING",     CALLBACK_TEMPLATE(on_exosip_notification_proceeding)},
		   {"EXOSIP_NOTIFICATION_ANSWERED",       CALLBACK_TEMPLATE(on_exosip_notification_answered)},
		   {"EXOSIP_NOTIFICATION_REDIRECTED",     CALLBACK_TEMPLATE(on_exosip_notification_redirected)},
		   {"EXOSIP_NOTIFICATION_REQUESTFAILURE", CALLBACK_TEMPLATE(on_exosip_notification_requestfailure)},
		   {"EXOSIP_NOTIFICATION_SERVERFAILURE",  CALLBACK_TEMPLATE(on_exosip_notification_serverfailure)},
		   {"EXOSIP_NOTIFICATION_GLOBALFAILURE",  CALLBACK_TEMPLATE(on_exosip_notification_globalfailure)}
	};

	for (uint32_t i = 0; i < EXOSIP_EVENT_COUNT; ++i)
	{
		_event_processor_map.insert(std::make_pair(i, eventProcTable[i]));
	}
}

EventHandlerManager::EventNameProcPair EventHandlerManager::GetEventProc(eXosip_event_type_t type)
{
	if (type > EXOSIP_EVENT_COUNT)
	{
		EventNameProcPair pair = { nullptr, nullptr };
		LOG(WARNING) << "Event Type: " << type << " don't exist!";
		return pair;
	}
	auto value = _event_processor_map.find(type);
	return value->second;
}

int EventHandlerManager::on_exosip_registration_success(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_registration_failure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_invite(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_reinvite(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_noanswer(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_proceeding(const SipEvent::Ptr& event)
{
	_call_handler.on_proceeding(event);

	return 0;
}

int EventHandlerManager::on_exosip_call_ringing(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_answered(const SipEvent::Ptr& event)
{
	_call_handler.HandleResponseSuccess(event);

	return 0;
}

int EventHandlerManager::on_exosip_call_redirected(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_requestfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_serverfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_globalfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_ack(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_cancelled(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_message_new(const SipEvent::Ptr& event)
{
	// ´òÓ¡message
	std::string reqid;
	osip_generic_param_t* tag = nullptr;
	//osip_to_get_tag(event->exosip_event->request->from, &tag);
	osip_uri_param_get_byname(&event->exosip_event->request->from->gen_params, (char*)"tag", &tag);
	if (nullptr == tag || nullptr == tag->gvalue) {
		reqid = "";
	}
	reqid = (const char*)tag->gvalue;

	LOG(INFO) << "on_exosip_call_message_new response reqid = " << reqid;

	if (!strncmp(event->exosip_event->request->sip_method, "MESSAGE", strlen("MESSAGE")))
	{
		_msg_handler.HandleIncomingRequest(event);
	}
	return 0;
}

int EventHandlerManager::on_exosip_call_message_proceeding(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_message_answered(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_message_redirected(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_message_requestfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_message_serverfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_message_globalfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_call_closed(const SipEvent::Ptr& event)
{
	_call_handler.HandleClose(event);
	return 0;
}

int EventHandlerManager::on_exosip_call_released(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_message_new(const SipEvent::Ptr& event)
{
	eXosip_event_t* exosip_event = event->exosip_event;

	if (MSG_IS_REGISTER(exosip_event->request)) {
		_register_handler.HandleIncomingRequest(event);
	}
	else if (MSG_IS_MESSAGE(exosip_event->request)) {
		_msg_handler.HandleIncomingRequest(event);
	}
	else if (MSG_IS_BYE(exosip_event->request)) {
		WarnL << " UNKNOW METHON   MSG_IS_BYE";
	}
	else {
		WarnL << " UNKNOW METHON";
	}

	LOG(INFO) << "===========================>>>" << exosip_event->request->sip_method;

	return 0;
}

int EventHandlerManager::on_exosip_message_proceeding(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_message_answered(const SipEvent::Ptr& event)
{
	//_msg_handler.HandleResponseSuccess(event);
	return 0;
}

int EventHandlerManager::on_exosip_message_redirected(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_message_requestfailure(const SipEvent::Ptr& event)
{
	_msg_handler.HandleResponseFailure(event);
	return 0;
}

int EventHandlerManager::on_exosip_message_serverfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_message_globalfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_noanswer(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_proceeding(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_answered(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_redirected(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_requestfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_serverfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_globalfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_subscription_notify(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_in_subscription_new(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_notification_noanswer(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_notification_proceeding(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_notification_answered(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_notification_redirected(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_notification_requestfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_notification_serverfailure(const SipEvent::Ptr& event)
{
	return 0;
}

int EventHandlerManager::on_exosip_notification_globalfailure(const SipEvent::Ptr& event)
{
	return 0;
}
