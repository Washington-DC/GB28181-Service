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
    int SendResponseAndGetAddress(const char* uname, eXosip_t* excontext, int tid, int status, std::string& address, uint16_t& port);
    int SendCallAck(struct eXosip_t* excontext, int did);
    int GetStatusCodeFromResponse(osip_message_t* response);
    std::string GetMsgIDFromRequest(osip_message_t* request);
};

//注册事件处理
class RegisterHandler:public BaseEventHandler
{
public:
    int HandleIncomingRequest(const SipEvent::Ptr& e);

private:
    void _response_register_401unauthorized(const SipEvent::Ptr& e);
};

//目录查询结果
class CatalogHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e,pugi::xml_document& doc);
};

//心跳消息处理
class HeartbeatHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);
};

//设备信息处理
class DeviceInfoHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);
};

//预置点查询信息
class PresetQueryHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);
};

//录像文件查询信息
class RecordQueryHandler :public BaseEventHandler
{
public:
    virtual bool Handle(const SipEvent::Ptr& e, pugi::xml_document& doc);
};

//其他消息处理
class MessageHandler :public BaseEventHandler
{
public:
    int HandleIncomingRequest(const SipEvent::Ptr& e);

    int HandleResponseSuccess(const SipEvent::Ptr& e);
    int HandleResponseFailure(const SipEvent::Ptr& e);
};

//播放事件处理
class CallHandler :public BaseEventHandler
{
public:
    int HandleResponseSuccess(const SipEvent::Ptr e);

    int on_proceeding(const SipEvent::Ptr e);

    int HandleClose(const SipEvent::Ptr e);
};