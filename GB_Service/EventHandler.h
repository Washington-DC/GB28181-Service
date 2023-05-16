#pragma once
#include "SipEvent.h"

class BaseEventHandler
{
public:
    typedef std::shared_ptr<BaseEventHandler> ptr;
    BaseEventHandler() = default;
    virtual ~BaseEventHandler() = default;

    //virtual int handle(SipEvent::Ptr, tinyxml2::XMLDocument& xml) {};

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

class MessageHandler :public BaseEventHandler
{
public:
    int HandleIncomingRequest(const SipEvent::Ptr& e);
};


class CallHandler :public BaseEventHandler
{

};