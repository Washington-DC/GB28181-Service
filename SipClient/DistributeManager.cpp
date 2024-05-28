#include "pch.h"
#include "DistributeManager.h"
#include "ConfigManager.h"
#include "HttpClient.h"

void DistributeManager::Start()
{
	auto&& items = ConfigManager::GetInstance()->GetAllDistributeItems();
	auto&& streams = HttpClient::GetInstance()->GetMediaList();

	for (auto&& s : streams)
	{
		auto iter = std::find_if(items.begin(), items.end(), [s](std::shared_ptr<DistributeItem> di) {
			return di->App == s.app && di->Stream == s.stream;
			});

		if (iter != items.end())
		{
			HttpClient::GetInstance()->AddDistributeStream(*iter);
		}
	}

}
