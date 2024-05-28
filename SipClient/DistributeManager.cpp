#include "pch.h"
#include "DistributeManager.h"
#include "ConfigManager.h"
#include "HttpClient.h"

void DistributeManager::Start()
{
	auto&& items = ConfigManager::GetInstance()->GetAllDistributeItems();
	auto&& streams = HttpClient::GetInstance()->GetMediaList();

	for (auto&& s : items)
	{
		auto iter = std::find_if(streams.begin(), streams.end(), [s](const media::mediaserver_stream_item& si) {
			return s->App == si.app && s->Stream == si.stream;
			});

		if (iter == streams.end())
		{
			HttpClient::GetInstance()->AddDistributeStream(s);
		}
	}

}
