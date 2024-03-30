#pragma once
#include "Structs.h"

class ConfigManager {
public:
    SINGLETON_DEFINE(ConfigManager);
    bool LoadConfig(std::string filepath);

    std::shared_ptr<SipServerInfo> GetSipServerInfo() { return server_info; }

    std::shared_ptr<MediaServerInfo> GetMediaServerInfo() { return media_server_info; }

    std::vector<std::shared_ptr<DeviceInfo>> GetAllDeviceInfo() { return devices; }

    int32_t GetHttpPort() { return http_port; }

private:
    ConfigManager() = default;

    std::shared_ptr<SipServerInfo> server_info = nullptr;//SIP服务器信息

    std::shared_ptr<MediaServerInfo> media_server_info = nullptr;//流媒体服务信息

    std::vector<std::shared_ptr<DeviceInfo>> devices;//所有的设备信息

    int32_t http_port = 28080;//HTTP服务使用的端口
};
