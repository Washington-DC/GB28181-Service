#pragma once

class DistributeManager
{
public:
	SINGLETON_DEFINE(DistributeManager);

	void Start();
	void Stop();

private:
	DistributeManager() = default;
	std::shared_ptr<std::thread> _thread = nullptr;

	std::mutex _mtx;
	std::condition_variable _cv;
	bool _start = false;
};

