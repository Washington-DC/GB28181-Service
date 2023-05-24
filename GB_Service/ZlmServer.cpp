#include "pch.h"
#include "ZlmServer.h"
#include "DTO.h"
#include "ConfigManager.h"
//#include <cpprest/http_client.h>
//using namespace web;
//using namespace utility;
//using namespace web::http;
//using namespace web::http::client;
//using namespace concurrency::streams;
//
//#ifdef _DEBUG
//#pragma comment(lib, "cpprest142_2_10d.lib")
//#else
//#pragma comment(lib, "cpprest142_2_10.lib")
//#endif

#include <cpr/cpr.h>

#pragma comment(lib,"libcurl-d_imp.lib")
#pragma comment(lib,"cpr-d.lib")

void ZlmServer::Init(std::shared_ptr<MediaServerInfo> info)
{
	_info = info;
	
	_base_url = fmt::format("http://{}:{}", _info->IP, _info->Port);
}


void ZlmServer::UpdateHeartbeatTime(time_t t)
{
	std::scoped_lock<std::mutex> g(_mutex);
	_last_heartbeat_time = t;

	if (_delay_task)
		_delay_task->cancel();

	_delay_task = toolkit::EventPollerPool::Instance().getPoller()->doDelayTask(1000 * 10, [this]()
		{
			auto now = time(nullptr);
			if (now - _last_heartbeat_time > 6)
				UpdateStatus(false);
			else
				UpdateStatus(true);
			return 0;
		});
}


void ZlmServer::UpdateStatus(bool flag)
{
	std::scoped_lock<std::mutex> g(_mutex);
	_connected = true;
}


bool ZlmServer::IsConnected()
{
	return _connected;
}


int ZlmServer::OpenRtpServer(const std::string& stream_id)
{
	auto uri = _base_url + "/index/api/openRtpServer";
	cpr::Response res = cpr::Get(cpr::Url{ uri },
		cpr::Parameters{
			{"secret",_info->Secret},
			{"port","0"},
			{"tcp_mode", "0"},
			{"stream_id",stream_id}
		}
	);

	if (res.status_code == 200)
	{
		auto info = nlohmann::json::parse(res.text).get<dto::ResponseInfo>();
		if (info.Code == 0)
		{
			return info.Port;
		}
	}
	return -1;
	//try {
	//	http_client_config config;
	//	config.set_timeout(6000ms);
	//	http_client client(_base_url, config);
	//	uri_builder builder(L"/index/api/openRtpServer");
	//	builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_info->Secret));
	//	builder.append_query(L"port", L"0");
	//	builder.append_query(L"tcp_mode", L"0");
	//	builder.append_query(L"stream_id", nbase::win32::MBCSToUnicode(stream_id));

	//	stringstreambuf buffer;
	//	LOG(INFO) << "OpenRtpServer: " << nbase::win32::UnicodeToMBCS(_base_url + builder.to_string());
	//	http_response response = client.request(methods::POST, builder.to_string()).get();
	//	response.body().read_to_end(buffer);
	//	std::string text = buffer.collection();
	//	LOG(INFO) << "Response: " << text;
	//	if (text.empty())
	//	{
	//		LOG(INFO) << "err";
	//	}
	//	auto info = nlohmann::json::parse(text).get<dto::ResponseInfo>();
	//	if (info.Code == 0)
	//	{
	//		auto ssrc = std::make_shared<SSRCInfo>(info.Port, _ssrc_config->GenerateSSRC(), stream_id);
	//		return ssrc;
	//	}
	//}
	//catch (const std::exception& e) {
	//	LOG(ERROR) << e.what();
	//}
}


void ZlmServer::CloseRtpServer(const std::string& stream_id)
{
	try {
		/*http_client_config config;
		config.set_timeout(3000ms);
		http_client client(_base_url, config);
		uri_builder builder(L"/index/api/closeRtpServer");
		builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_info->Secret));
		builder.append_query(L"stream_id", nbase::win32::MBCSToUnicode(stream_id));

		stringstreambuf buffer;
		http_response response = client.request(methods::GET, builder.to_string()).get();*/

	}
	catch (const std::exception&) {
	}
}


std::string ZlmServer::ListRtpServer()
{
	try {
		/*http_client_config config;
		config.set_timeout(3000ms);
		http_client client(_base_url, config);
		uri_builder builder(L"/index/api/listRtpServer");
		builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_info->Secret));

		stringstreambuf buffer;
		http_response response = client.request(methods::GET, builder.to_string()).get();

		return buffer.collection();*/

	}
	catch (const std::exception&) {
	}

	return "";
}
