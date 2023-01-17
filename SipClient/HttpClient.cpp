#include "pch.h"
#include "HttpClient.h"


void HttpClient::Init(std::shared_ptr<MediaServerInfo> info)
{
	this->_server_info = info;
	_base_url = fmt::format(L"http://{}:{}", nbase::win32::MBCSToUnicode(_server_info->IP), _server_info->Port);
}


bool HttpClient::StartSendRtp(std::shared_ptr<ChannelInfo> channel_info, std::string ssrc, std::string dst_ip, int dst_port, int local_port, bool use_tcp)
{
	try
	{
		http_client_config config;
		config.set_timeout(3000ms);
		http_client client(_base_url, config);
		uri_builder builder(L"/index/api/startSendRtp");

		builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_server_info->Secret));
		builder.append_query(L"vhost", L"__defaultVhost__");
		builder.append_query(L"app", nbase::win32::MBCSToUnicode(channel_info->App));
		builder.append_query(L"stream", nbase::win32::MBCSToUnicode(channel_info->Stream));
		builder.append_query(L"ssrc", nbase::win32::MBCSToUnicode(ssrc));
		builder.append_query(L"dst_url", nbase::win32::MBCSToUnicode(dst_ip));
		builder.append_query(L"dst_port", std::to_wstring(dst_port));
		builder.append_query(L"src_port", std::to_wstring(local_port));
		builder.append_query(L"is_udp", use_tcp ? L"0" : L"1");
		LOG(INFO) << "发送请求: " << nbase::win32::UnicodeToMBCS(builder.to_string());

		stringstreambuf buffer;
		http_response response = client.request(methods::GET, builder.to_string()).get();
		response.body().read_to_end(buffer);

		auto text = buffer.collection();
		LOG(INFO) << "返回:" << text;
		if (!text.empty())
		{
			auto doc = nlohmann::json::parse(text);
			if (!doc["code"].empty())
			{
				auto status = doc["code"].get<int>();
				if (status == 0)
				{
					return true;
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << e.what();
	}
	return false;
}


bool HttpClient::StopSendRtp(std::shared_ptr<ChannelInfo> channel_info)
{
	try
	{
		http_client_config config;
		config.set_timeout(3000ms);
		http_client client(_base_url, config);
		uri_builder builder(L"/index/api/stopSendRtp");
		builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_server_info->Secret));
		builder.append_query(L"vhost", L"__defaultVhost__");
		builder.append_query(L"app", L"h265");
		builder.append_query(L"stream", L"ch2/sub/av_stream");

		stringstreambuf buffer;
		http_response response = client.request(methods::GET, builder.to_string()).get();
		auto text = buffer.collection();
		LOG(INFO) << "返回:" << text;
	}
	catch (const std::exception&)
	{

	}
	return false;
}