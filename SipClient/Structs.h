#pragma once
#include "pch.h"

/// @brief SIP服务器信息
struct SipServerInfo {
	std::string IP;     //服务端IP 
	int Port;           //服务端SIP端口
	std::string ID;     //服务器ID
	std::string Password;//密码
};

/// @brief 流媒体服务信息
struct MediaServerInfo {
	std::string IP;     //流媒体服务器信息
	int Port;           //流媒体服务HTTP端口
	std::string Secret; //流媒体服务器校验码
};

/// @brief 通道信息
struct ChannelInfo {
	std::string ID;     //通道ID
	std::string Name;   //通道名称
	std::string App;    //通道对应的视频流路径
	std::string Stream; //通道对应的视频流路径
};

/// @brief 设备信息
struct DeviceInfo {
	std::string IP;     //设备IP，也就是计算机IP，这个字段不需要
	int Port;           //SIP通信端口，0：则随机选择一个端口，其他则会使用指定的端口，根据需要来设置
	std::string ID;     //设备ID
	int Protocol;       //sip通信协议，TCP or UDP
	std::string Name;   //设备名称
	std::string Manufacturer;   //设备厂家
	int HeartbeatInterval;      //心跳周期，单位s
	bool CloseAllWhenBye = false; //当收到bye命令时，结束所有推流

	std::vector<std::shared_ptr<ChannelInfo>> Channels; //此设备包含的所有通道
};

/// @brief 播放会话信息
struct SessionInfo {
	int32_t DialogID; //会话ID
	std::string SSRC;   // 媒体标识符，使用此字段区分不同的媒体流
	std::string TargetIP;   //目的IP,也就是rtp数据需要向这个IP发送
	int TargetPort;         //目的端口,rtp数据需要向这个port发送
	int LocalPort;          //本地端口，本地发送rtp数据的端口
	bool UseTcp = false;    //使用TCP协议？

	bool Playback = false;  //是否为回放
	int64_t StartTime;  //回放开始时间,unix时间戳
	int64_t EndTime;    //回放结束时间,unix时间戳

	std::string FilePath;   //MP4文件路径

	bool Used = false;

	std::shared_ptr<ChannelInfo> Channel = nullptr; //对应的通道信息

	std::string ToString() {
		return fmt::format(
			"DialogID: {}\nID: {}\nSSRC: {}\nDstIP: {}\nDstPort: {}\nLocalPort: {}", DialogID, Channel->ID, SSRC, TargetIP, TargetPort, LocalPort);
	}
};

/// @brief 拉流分发配置
struct DistributeItem
{
	std::string Source;  //源地址
	std::string App;
	std::string Stream;
	int Protocol; //针对RTSP时，使用TCP:0 or UDP:1，默认TCP:0
	bool RecordMP4 = false;
	int RetryTimes = -1;  //无限重试
};
