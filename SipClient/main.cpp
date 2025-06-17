#include "ConfigManager.h"
#include "DbManager.h"
#include "Device.h"
#include "DistributeManager.h"
#include "HttpClient.h"
#include "HttpServer.h"
#include "Utils.h"
#include "pch.h"

int main() {
    auto root = fs::path(GetCurrentModuleDirectory());
    auto log_path = root / "logs";
    auto config_file = root / "config.xml";
    auto db_file = root / "record.db";

    fs::create_directories(log_path);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_path = log_path / fmt::format("{:%Y%m%d%H%M%S}.log", fmt::localtime(std::time(nullptr)));
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file_path.string(), 1024 * 1024 * 50, 12, true);
    auto logger = std::make_shared<spdlog::logger>("logger", spdlog::sinks_init_list { console_sink, file_sink });

#if defined(_WIN32) && defined(_MSC_VER)
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    logger->sinks().push_back(msvc_sink);
#endif

    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%^%l%$] [%t] [%s:%#] %v");
    spdlog::set_default_logger(logger);
    logger->set_level(spdlog::level::trace);

    //加载配置文件
    auto ret = ConfigManager::GetInstance()->LoadConfig(config_file.string());
    if (!ret)
        return 0;

    auto sip_server_info = ConfigManager::GetInstance()->GetSipServerInfo();
    auto media_server_info = ConfigManager::GetInstance()->GetMediaServerInfo();
    auto device_infos = ConfigManager::GetInstance()->GetAllDeviceInfo();

    DbManager::GetInstance()->Init(db_file.string());
    HttpClient::GetInstance()->Init(media_server_info);

    //检查拉流分发参数
    auto &&items = ConfigManager::GetInstance()->GetAllDistributeItems();
    if (!items.empty()) {
        for (auto &&item : items) {
            auto table_name = item->DbName();
            DbManager::GetInstance()->CreateTable(table_name);
        }

        DistributeManager::GetInstance()->Start();
    }

    //设备初始化
    std::vector<std::shared_ptr<SipDevice>> devices;
    for (auto &&info : device_infos) {
        auto device = std::make_shared<SipDevice>(info, sip_server_info);
        device->Init();

        for (auto &&channel : info->Channels) {
            auto table_name = channel->DbName();
            DbManager::GetInstance()->CreateTable(table_name);
        }

        device->StartSipClient();
        devices.push_back(device);
    }

    //启动HTTP服务
    auto f = HttpServer::GetInstance()->Start(ConfigManager::GetInstance()->GetHttpPort());

#ifdef _DEBUG
    getchar();
#else
    static std::promise<void> s_exit;
    signal(SIGINT, [](int signal) {
        SPDLOG_INFO("Catch Signal: {}", signal);
        s_exit.set_value();
    });
    s_exit.get_future().wait();
#endif

    for (auto &&device : devices) {
        device->Logout();
        device->StopSipClient();
    }

    DistributeManager::GetInstance()->Stop();

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
