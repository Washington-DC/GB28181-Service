#pragma once
#include "SipEvent.h"
#include "EventHandler.h"

class EventHandlerManager
{
public:

    typedef struct {
        const char* name;
        SipEvent::event_proc  proc;
    } EventNameProcPair;

    EventHandlerManager();

    EventNameProcPair GetEventProc(eXosip_event_type_t type);

public:
    /* REGISTER related events */
    int on_exosip_registration_success(const SipEvent::Ptr& event);
    int on_exosip_registration_failure(const SipEvent::Ptr& event);


public:
    /* INVITE related events within calls */
    int on_exosip_call_invite(const SipEvent::Ptr& event);
    int on_exosip_call_reinvite(const SipEvent::Ptr& event);
    int on_exosip_call_noanswer(const SipEvent::Ptr& event);
    int on_exosip_call_proceeding(const SipEvent::Ptr& event);
    int on_exosip_call_ringing(const SipEvent::Ptr& event);
    int on_exosip_call_answered(const SipEvent::Ptr& event);
    int on_exosip_call_redirected(const SipEvent::Ptr& event);
    int on_exosip_call_requestfailure(const SipEvent::Ptr& event);
    int on_exosip_call_serverfailure(const SipEvent::Ptr& event);
    int on_exosip_call_globalfailure(const SipEvent::Ptr& event);
    int on_exosip_call_ack(const SipEvent::Ptr& event);
    int on_exosip_call_cancelled(const SipEvent::Ptr& event);

public:
    /* request related events within calls (except INVITE) */
    int on_exosip_call_message_new(const SipEvent::Ptr& event);
    int on_exosip_call_message_proceeding(const SipEvent::Ptr& event);
    int on_exosip_call_message_answered(const SipEvent::Ptr& event);
    int on_exosip_call_message_redirected(const SipEvent::Ptr& event);
    int on_exosip_call_message_requestfailure(const SipEvent::Ptr& event);
    int on_exosip_call_message_serverfailure(const SipEvent::Ptr& event);
    int on_exosip_call_message_globalfailure(const SipEvent::Ptr& event);
    int on_exosip_call_closed(const SipEvent::Ptr& event);

public:
    /* for both UAS & UAC events */
    int on_exosip_call_released(const SipEvent::Ptr& event);

public:
    /* events received for request outside calls */
    int on_exosip_message_new(const SipEvent::Ptr& event);
    int on_exosip_message_proceeding(const SipEvent::Ptr& event);
    int on_exosip_message_answered(const SipEvent::Ptr& event);
    int on_exosip_message_redirected(const SipEvent::Ptr& event);
    int on_exosip_message_requestfailure(const SipEvent::Ptr& event);
    int on_exosip_message_serverfailure(const SipEvent::Ptr& event);
    int on_exosip_message_globalfailure(const SipEvent::Ptr& event);

public:
    /* Presence and Instant Messaging */
    int on_exosip_subscription_noanswer(const SipEvent::Ptr& event);
    int on_exosip_subscription_proceeding(const SipEvent::Ptr& event);
    int on_exosip_subscription_answered(const SipEvent::Ptr& event);
    int on_exosip_subscription_redirected(const SipEvent::Ptr& event);
    int on_exosip_subscription_requestfailure(const SipEvent::Ptr& event);
    int on_exosip_subscription_serverfailure(const SipEvent::Ptr& event);
    int on_exosip_subscription_globalfailure(const SipEvent::Ptr& event);
    int on_exosip_subscription_notify(const SipEvent::Ptr& event);

public:
    int on_exosip_in_subscription_new(const SipEvent::Ptr& event);

public:
    int on_exosip_notification_noanswer(const SipEvent::Ptr& event);
    int on_exosip_notification_proceeding(const SipEvent::Ptr& event);
    int on_exosip_notification_answered(const SipEvent::Ptr& event);
    int on_exosip_notification_redirected(const SipEvent::Ptr& event);
    int on_exosip_notification_requestfailure(const SipEvent::Ptr& event);
    int on_exosip_notification_serverfailure(const SipEvent::Ptr& event);
    int on_exosip_notification_globalfailure(const SipEvent::Ptr& event);


private:
    std::map<uint32_t, EventNameProcPair> _event_processor_map;
    MessageHandler                        _msg_handler;
    RegisterHandler                       _register_handler;
    CallHandler                           _call_handler;
};

