#pragma once
#include "Structs.h"

class HttpClient {
public:
    SINGLETON_DEFINE(HttpClient);

    void Init(std::shared_ptr<MediaServerInfo> server_info);

    bool StartSendRtp(
        std::shared_ptr<ChannelInfo> info, std::string ssrc, std::string dst_ip, int dst_port, int local_port,
        bool use_tcp = false);
    //bool StopSendRtp(std::shared_ptr<ChannelInfo> info);
    bool StopSendRtp(std::shared_ptr<SessionInfo> info);

    bool StartSendPlaybackRtp(
        std::shared_ptr<ChannelInfo> channel_info, std::string ssrc, std::string dst_ip,
        int dst_port, int local_port, std::string& start_time, std::string& end_time, bool use_tcp);

    bool GetMp4RecordInfo(std::string stream,
        std::string start_time, std::string end_time, std::string& response);

    bool SetPause(std::string app, std::string stream, bool pause);
    bool SetSpeed(std::string app, std::string stream, float speed);

private:
    std::shared_ptr<MediaServerInfo> _server_info = nullptr;
    //std::wstring _base_url = L"http://127.0.0.1:8000";
    std::string _base_url = "http://127.0.0.1:8000";

private:
    HttpClient() = default;
};