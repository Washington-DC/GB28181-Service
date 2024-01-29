#pragma once
#include "pch.h"

struct SipServerInfo {
    std::string IP;
    int Port;
    std::string ID;
    std::string Password;
};

struct MediaServerInfo {
    std::string IP;
    int Port;
    std::string Secret;
};

struct ChannelInfo {
    std::string ID;
    std::string Name;
    std::string App;
    std::string Stream;
};

struct DeviceInfo {
    std::string IP;
    int Port;
    std::string ID;
    int Protocol;
    std::string Name;
    std::string Manufacturer;
    int HeartbeatInterval;

    std::vector<std::shared_ptr<ChannelInfo>> Channels;
};

struct SessionInfo {
    int32_t DialogID; //会话ID
    std::string ID;
    std::string SSRC;
    std::string TargetIP;
    int TargetPort;
    int LocalPort;
    bool UseTcp = false;

    bool Playback = false;
    std::string StartTime;
    std::string EndTime;

    std::shared_ptr<ChannelInfo> Channel = nullptr;

    std::string to_string() {
        return fmt::format(
            "DialogID:{}\nID:{}\nSSRC:{}\nDstIP:{}\nDstPort:{}\nLocalPort:{}", DialogID, ID, SSRC, TargetIP, TargetPort, LocalPort);
    }
};
