// SipService.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "ConfigManager.h"
#include "HttpClient.h"
#include "Device.h"

int main()
{
	google::InitGoogleLogging("");
	google::SetStderrLogging(google::GLOG_INFO);
	FLAGS_logbufsecs = 1;
	FLAGS_colorlogtostderr = true;

	auto root = nbase::win32::GetCurrentModuleDirectory();
	auto config_file = root + L"config.xml";

	auto ret = ConfigManager::GetInstance()->LoadConfig(config_file);
	if (!ret)
		return 0;

	auto sip_server_info = ConfigManager::GetInstance()->GetSipServerInfo();
	auto media_server_info = ConfigManager::GetInstance()->GetMediaServerInfo();
	auto device_infos = ConfigManager::GetInstance()->GetAllDeviceInfo();
	HttpClient::GetInstance()->Init(media_server_info);
	std::vector<std::shared_ptr<SipDevice>> devices;
	for (auto&& info : device_infos)
	{
		auto device = std::make_shared<SipDevice>(info, sip_server_info);
		device->init();
		device->start_sip_client();
		devices.push_back(device);
	}

	getchar();

	for (auto&& device : devices)
	{
		device->log_out();
		device->stop_sip_client();
	}

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
