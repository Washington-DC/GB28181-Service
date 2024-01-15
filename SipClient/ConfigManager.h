#pragma once
#include "Structs.h"

class ConfigManager {
public:
    SINGLETON_DEFINE(ConfigManager);
    bool LoadConfig(std::wstring filepath);

    std::shared_ptr<SipServerInfo> GetSipServerInfo() { return server_info; }

    std::shared_ptr<MediaServerInfo> GetMediaServerInfo() { return media_server_info; }

    std::vector<std::shared_ptr<DeviceInfo>> GetAllDeviceInfo() { return devices; }

    int32_t GetHttpPort() { return http_port; }

private:
    ConfigManager() = default;

    std::shared_ptr<SipServerInfo> server_info = nullptr;
    std::shared_ptr<MediaServerInfo> media_server_info = nullptr;
    std::vector<std::shared_ptr<DeviceInfo>> devices;
    int32_t http_port = 28080;
};
