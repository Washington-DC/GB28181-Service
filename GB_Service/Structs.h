#pragma once
#include "pch.h"

struct SipServerInfo {
    std::string IP;
    int Port;
    std::string ID;
    std::string Domain;
    std::string Password;
    std::string Nonce;
};

struct MediaServerInfo {
    std::string IP;
    int Port;
    std::string Secret;
};
//
//struct ChannelInfo {
//    std::string ID;
//    std::string Name;
//    std::string App;
//    std::string Stream;
//};
//
//struct DeviceInfo {
//    std::string IP;
//    int Port;
//    std::string ID;
//    IPPROTO Protocol;
//    std::string Name;
//    std::string Manufacturer;
//    int HeartbeatInterval;
//
//    std::vector<std::shared_ptr<ChannelInfo>> Channels;
//};
//
//struct SessinInfo {
//    std::string ID;
//    std::string SSRC;
//    std::string TargetIP;
//    int TargetPort;
//    int LocalPort;
//    bool UseTcp = false;
//    std::shared_ptr<ChannelInfo> Channel = nullptr;
//
//    std::string to_string() {
//        return fmt::format(
//            "ID:{}\nSSRC:{}\nDstIP:{}\nDstPort:{}\nLocalPort:{}", ID, SSRC, TargetIP, TargetPort, LocalPort);
//    }
//};



enum REQUEST_MESSAGE_TYPE
{
    REQUEST_TYPE_UNKNOWN = 0,
    KEEPALIVE,                   // 保活心跳
    QUIRY_CATALOG,               //   查询目录
    DEVICE_CONTROL_PTZ,         // 设备控制-云台
    DEVICE_QUIER_PRESET,        // 设备查询-预置位
    DEVICE_CONTROL_PRESET,       // 设备控制-预置位
    DEVICE_CONTROL_HOMEPOSITION, // 设备控制-看守位

    REQUEST_CALL_INVITE,            // 点播
    REQUEST_CALL_PLAYBACK,          // 回放
    REQUEST_CALL_LIVE,              // 直播
    REQUEST_CALL_DOWNLOAD,          // 下载
    REQUEST_CALL_BYE,               // 挂断

    REQUEST_TYPE_MAX
};


enum REQ_CALL_TYPE
{
    REQ_CALL_TYPE_UNKNOWN = 0,

    REQ_CALL_TYPE_MAX
};
