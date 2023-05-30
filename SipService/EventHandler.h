#pragma once
#include "SipEvent.h"

class BaseEventHandler
{
public:
    typedef std::shared_ptr<BaseEventHandler> ptr;
    BaseEventHandler() = default;
    virtual ~BaseEventHandler() = default;

    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);

protected:
    int SendResponse(const char* uname, struct eXosip_t* excontext, int tid, int status);
    int SendCallAck(struct eXosip_t* excontext, int did);
    int GetStatusCodeFromResponse(osip_message_t* response);
    std::string GetMsgIDFromRequest(osip_message_t* request);
};

class RegisterHandler:public BaseEventHandler
{
public:
    int HandleIncomingRequest(const SipEvent::Ptr& e);

private:
    void _response_register_401unauthorized(const SipEvent::Ptr& e);
};


class CatalogHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e,pugi::xml_document& doc);
};


class HeartbeatHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);
};

class DeviceInfoHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);
};

class PresetQueryHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);
};


class MessageHandler :public BaseEventHandler
{
public:
    int HandleIncomingRequest(const SipEvent::Ptr& e);

    int HandleResponseSuccess(const SipEvent::Ptr& e);
    int HandleResponseFailure(const SipEvent::Ptr& e);
};


class CallHandler :public BaseEventHandler
{
public:
    int HandleResponseSuccess(const SipEvent::Ptr e);

    int on_proceeding(const SipEvent::Ptr e);

    int HandleClose(const SipEvent::Ptr e);
};