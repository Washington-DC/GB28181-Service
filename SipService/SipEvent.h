#pragma once

class SipEvent :public std::enable_shared_from_this<SipEvent>
{
public:
    typedef std::shared_ptr<SipEvent> Ptr;
    typedef std::function<int(const SipEvent::Ptr&)> event_proc;

public:
    int                 value;      // 事件值
    const char*         name;       // 事件名称
    event_proc          proc;       // 事件处理函数
    struct eXosip_t*    exosip_context;  // eXosip上下文
    eXosip_event_t*     exosip_event;    // eXosip事件
    uint64_t            id;         // 事件id
};

