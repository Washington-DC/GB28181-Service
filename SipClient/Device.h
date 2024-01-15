#pragma once
#include "Structs.h"

using event_proc = std::function<void(eXosip_event_t*)> ;

class SipDevice : public DeviceInfo {
public:
    SipDevice(const std::shared_ptr<DeviceInfo> info, std::shared_ptr<SipServerInfo> sip_server_info);

public:
    bool init();
    bool start_sip_client();
    bool stop_sip_client();
    bool log_out();
    bool send_notify(std::shared_ptr<SessionInfo> session);

private:
    void on_response();

    void on_registration_failure(eXosip_event_t *event);
    void on_registration_success(eXosip_event_t *event);
    void on_message_new(eXosip_event_t *event);
    void on_message_request_failure(eXosip_event_t *event);
    void on_call_ack(eXosip_event_t *event);
    void on_call_closed(eXosip_event_t *event);
    void on_call_invite(eXosip_event_t *event);
    void on_call_message_new(eXosip_event_t* event);
    void on_in_subscription_new(eXosip_event_t* event);

    void send_response_ok(eXosip_event_t *event);
    void heartbeat_task();
    void mobile_position_task();

    std::string generate_catalog_xml(const std::string &sn);
    std::string parse_ssrc(std::string text);
    bool parse_time(std::string text, std::string& start_time, std::string& end_time);
    std::shared_ptr<ChannelInfo> find_channel(std::string id);

private:
    std::shared_ptr<SipServerInfo> _sip_server_info = nullptr;

    std::string format_xml(std::string& text);

private:
    eXosip_t *_sip_context = nullptr;
    int _local_port = 50000;

private:
    std::string _from_uri = "";
    std::string _contact_url = "";
    std::string _proxy_uri = "";

    std::string _sip_server_domain = "";

    int _register_id = -1;
    bool _logout = false;
    bool _is_running = false;
    bool _is_heartbeat_running = false;
    bool _register_success = false;

    int32_t _subscription_dialog_id = -1;
    std::mutex _subscription_mutex;
    std::condition_variable _subscription_condition;

    std::mutex _heartbeat_mutex;
    std::condition_variable _heartbeat_condition;

    std::mutex _session_mutex;
    std::unordered_map<int32_t, std::shared_ptr<SessionInfo>> _session_map;
    std::unordered_map<eXosip_event_type_t, event_proc> _event_processor_map;

private:
    std::shared_ptr<std::thread> _sip_thread = nullptr;
    std::shared_ptr<std::thread> _heartbeat_thread = nullptr;
    std::shared_ptr<std::thread> _subscription_thread = nullptr;
};

