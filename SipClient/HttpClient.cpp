#include "pch.h"
#include "HttpClient.h"

void HttpClient::Init(std::shared_ptr<MediaServerInfo> info) {
	this->_server_info = info;
	_base_url = fmt::format(L"http://{}:{}", nbase::win32::MBCSToUnicode(_server_info->IP), _server_info->Port);
}

bool HttpClient::StartSendRtp(
	std::shared_ptr<ChannelInfo> channel_info, std::string ssrc, std::string dst_ip, int dst_port, int local_port, bool use_tcp) {

	stringstreambuf buffer;
	pplx::task<void> request_task = buffer.sync()
		.then([this, channel_info, ssrc, dst_ip, dst_port, local_port, use_tcp]()
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

				  LOG(INFO) << "·¢ËÍÇëÇó: " << nbase::win32::UnicodeToMBCS(builder.to_string());
				  return client.request(methods::GET, builder.to_string());
			  })
		.then([this, buffer](http_response response)
			  {
				  return response.body().read_to_end(buffer);
			  })
				  .then([this, buffer](size_t size)
						{
							auto text = buffer.collection();
							LOG(INFO) << "·µ»Ø:" << text;
							/*if (!text.empty()) {
								auto doc = nlohmann::json::parse(text);
								if (!doc["code"].empty()) {
									auto status = doc["code"].get<int>();
									if (status == 0) {
										return true;
									}
								}
							}*/
						});

			  try
			  {
				  request_task.wait();
				  return true;
			  }
			  catch (const std::exception& e)
			  {
				  LOG(ERROR) << e.what();
			  }
			  return false;
}

bool HttpClient::StopSendRtp(std::shared_ptr<ChannelInfo> channel_info) {
	try {
		http_client_config config;
		config.set_timeout(3000ms);
		http_client client(_base_url, config);
		uri_builder builder(L"/index/api/stopSendRtp");
		builder.append_query(L"secret", nbase::win32::MBCSToUnicode(_server_info->Secret));
		builder.append_query(L"vhost", L"__defaultVhost__");
		builder.append_query(L"app", nbase::win32::MBCSToUnicode(channel_info->App));
		builder.append_query(L"stream", nbase::win32::MBCSToUnicode(channel_info->Stream));

		stringstreambuf buffer;
		http_response response = client.request(methods::GET, builder.to_string()).get();

		return true;
	}
	catch (const std::exception&) {
	}
	return false;
}