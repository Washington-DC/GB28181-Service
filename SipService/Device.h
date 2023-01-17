#pragma once
#include "Structs.h"

class SipDevice :public DeviceInfo
{
public:
	SipDevice(const std::shared_ptr<DeviceInfo> info,
			  std::shared_ptr<SipServerInfo> sip_server_info,
			  std::shared_ptr<MediaServerInfo> media_server_info);

public:
	bool init();
	bool start_sip_client();
	bool stop_sip_client();
	bool log_out();
	bool send_device_info();

private:

	void on_response();

	void on_registration_failure(eXosip_event_t* event);
	void on_registration_success(eXosip_event_t* event);
	void on_message_new(eXosip_event_t* event);
	void on_message_request_failure(eXosip_event_t* event);
	void on_call_ack(eXosip_event_t* event);
	void on_call_closed(eXosip_event_t* event);
	void on_call_invite(eXosip_event_t* event);

	void send_response_ok(eXosip_event_t* event);
	void heartbeat_task();

	std::string generate_catalog_xml(const std::string& sn);

private:
	std::shared_ptr<SipServerInfo> _sip_server_info = nullptr;
	std::shared_ptr<MediaServerInfo> _media_server_info = nullptr;

private:
	eXosip_t* _sip_context = nullptr;
	int _local_port = 50000;

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

	std::string _invite_video_channel_id = "";
	std::shared_ptr<ChannelInfo> _invite_channel_info = nullptr;

	std::wstring _baseurl = L"http://127.0.0.1:18880";

	std::mutex _heartbeat_mutex;
	std::condition_variable _heartbeat_condition;

	pugi::xml_document _xml_doc_catalog;

private:
	std::shared_ptr<std::thread> _sip_thread = nullptr;
	std::shared_ptr<std::thread> _heartbeat_thread = nullptr;
};