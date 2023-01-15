#pragma once

#include <eXosip2/eXosip.h>

class SipDevice
{
public:
	SipDevice(std::string local_ip, int local_port, std::string device_sip_id, std::string server_ip, int server_port, std::string server_sip_id, std::string password);
	void init();
	bool start_sip_client(int local_port);;
	bool stop_sip_client();
	bool log_out();

private:

	void on_response();

	void on_registration_failure(eXosip_event_t* event);
	void on_registration_success(eXosip_event_t* event);
	void on_message_new(eXosip_event_t* event);
	void on_message_request_failure(eXosip_event_t* event);
	void on_call_ack(eXosip_event_t* event);
	void on_call_closed(eXosip_event_t* event);
	void on_call_invite(eXosip_event_t* event);
	void on_in_subscription_new(eXosip_event_t* event);

	void send_response_ok(eXosip_event_t* event);
	void heartbeat_task();
private:
	eXosip_t* _sip_context = nullptr;

	std::string _local_ip = "";
	std::string _server_ip = "";

	int _local_port = 50000;
	int _server_port = 15060;

	std::string _server_sip_id = "34020000002000000001";
	std::string _device_sip_id = "34020000002000000001";
	std::string _video_channel_id = "34020000002000000001";

	std::string _password = "12345678";

private:
	std::string _from_uri = "";
	std::string _contact_url = "";
	std::string _proxy_uri = "";

	bool _register_success = false;
	int _register_id = -1;

	std::string _ssrc = "";

	int _call_id = -1;
	int _dialog_id = -1;

	std::string _target_ip = "";
	int _target_port = -1;
	int _listen_port = -1;
	bool _use_tcp = false;
	bool _logout = false;

	bool _is_running = false;
	bool _is_heartbeat_running = false;
	bool _is_pushing_stream = false;

	std::wstring baseurl = L"http://127.0.0.1:18880";

	std::mutex _heartbeat_mutex;
	std::condition_variable _heartbeat_condition;

private:
	std::shared_ptr<std::thread> sip_thread = nullptr;
	std::shared_ptr<std::thread> _heartbeat_thread = nullptr;
};

