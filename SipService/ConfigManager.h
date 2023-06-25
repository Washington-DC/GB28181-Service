#pragma once
#include "Structs.h"

class ConfigManager {
public:
    SINGLETON_DEFINE(ConfigManager);
    bool LoadConfig(std::string filepath);

    std::shared_ptr<SipServerInfo> GetSipServerInfo() { return server_info; }
    std::shared_ptr<MediaServerInfo> GetMediaServerInfo() { return media_server_info; }
    int GetHttpPort() { return http_port; }
private:
    ConfigManager() = default;

    int http_port = 8000;
    std::shared_ptr<SipServerInfo> server_info = nullptr;
    std::shared_ptr<MediaServerInfo> media_server_info = nullptr;
};
