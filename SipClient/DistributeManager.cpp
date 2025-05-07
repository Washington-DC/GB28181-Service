#include "pch.h"
#include "DistributeManager.h"
#include "ConfigManager.h"
#include "HttpClient.h"

void DistributeManager::Start()
{
	_start = true;
	_thread = std::make_shared<std::thread>([this]()
		{
			while (_start)
			{
				//配置的拉流分发项
				auto&& items = ConfigManager::GetInstance()->GetAllDistributeItems();
				//当前服务器的媒体流信息
				auto&& streams = HttpClient::GetInstance()->GetMediaList();

				for (auto&& s : items)
				{
					auto iter = std::find_if(streams.begin(), streams.end(), [s](const media::mediaserver_stream_item& si) {
						return s->App == si.app && s->Stream == si.stream;
						});

					if (iter == streams.end())
					{
						//不存在，则添加拉流代理
						HttpClient::GetInstance()->AddDistributeStream(s);
					}
					else
					{
						//如果设置了录制，但是这里没有录制的话
						if (!iter->isRecordingMP4 && s->RecordMP4)
						{
							HttpClient::GetInstance()->StartRecord(s->App, s->Stream);
						}

						if (iter->isRecordingMP4 && !s->RecordMP4)
						{
							HttpClient::GetInstance()->StopRecord(s->App, s->Stream);
						}
					}
				}

				//定时
				std::unique_lock<std::mutex> lck(_mtx);
				_cv.wait_for(lck, std::chrono::seconds(30s));
			};
		}
	);
}

void DistributeManager::Stop()
{
	_start = false;
	_cv.notify_all();

	if (_thread && _thread->joinable())
	{
		_thread->join();
		_thread = nullptr;
	}
}
