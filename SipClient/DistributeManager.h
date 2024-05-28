#pragma once

class DistributeManager
{
public:
	SINGLETON_DEFINE(DistributeManager);

	void Start();

private:
	DistributeManager() = default;

	std::mutex _mtx;
	std::condition_variable _cv;
};

