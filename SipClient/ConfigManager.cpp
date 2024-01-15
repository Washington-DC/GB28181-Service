#include "pch.h"
#include "ConfigManager.h"

bool ConfigManager::LoadConfig(std::wstring filepath) {
	LOG(INFO) << "配置文件解析...";
	pugi::xml_document doc;
	auto ret = doc.load_file(filepath.c_str(), pugi::parse_full);
	if (ret.status != pugi::status_ok) {
		LOG(ERROR) << "配置文件解析失败";
		return false;
	}

	auto root = doc.child("Config");
	// SIP 服务器
	auto sip_server_node = root.child("SipServer");
	if (sip_server_node) {
		server_info = std::make_shared<SipServerInfo>();
		// SIP 服务器IP
		server_info->IP = sip_server_node.child_value("IP");
		// SIP 服务器固定端口
		nbase::StringToInt(sip_server_node.child_value("Port"), &server_info->Port);
		// SIP 服务器ID
		server_info->ID = sip_server_node.child_value("ID");
		// SIP 服务器固定密码
		server_info->Password = sip_server_node.child_value("Password");
	}
	else {
		LOG(ERROR) << "SipServer节点错误";
		return false;
	}

	//流媒体服务
	auto media_server_node = root.child("MediaServer");
	if (media_server_node) {
		media_server_info = std::make_shared<MediaServerInfo>();

		//流媒体服务 IP
		media_server_info->IP = media_server_node.child_value("IP");
		//流媒体服务 端口
		nbase::StringToInt(media_server_node.child_value("Port"), &media_server_info->Port);
		//流媒体服务 如果不是127.0.0.1的话，需要校验Secret字段
		media_server_info->Secret = media_server_node.child_value("Secret");
	}
	else {
		LOG(ERROR) << "MediaServer节点错误";
		return false;
	}


	//设备列表，当前软件的主要工作内容
	auto http_server_node = root.child("HttpServer");
	if (http_server_node) {
		nbase::StringToInt(http_server_node.child_value("Port"), &http_port);
	}

	//设备列表，当前软件的主要工作内容
	auto devicelist_node = root.child("DeviceList");
	if (devicelist_node) {
		auto device_nodes = devicelist_node.children("Device");
		for (auto&& device_node : device_nodes) {
			auto device_info = std::make_shared<DeviceInfo>();

			device_info->IP = device_node.child_value("IP");
			nbase::StringToInt(device_node.child_value("Port"), &device_info->Port);
			//当前设备ID
			device_info->ID = device_node.child_value("ID");
			//设备名称
			device_info->Name = device_node.child_value("Name");
			//厂家
			device_info->Manufacturer = device_node.child_value("Manufacturer");
			// SIP传输协议
			std::string text = device_node.child_value("Protocol");
			device_info->Protocol = (text.compare("TCP") == 0 ? IPPROTO_TCP : IPPROTO_UDP);
			//心跳间隔时间
			nbase::StringToInt(device_node.child_value("HeartbeatInterval"), &device_info->HeartbeatInterval);
			//设备目录，一般情况下只有一个通道，代表一个设备， 不排除某些情况下会有多台设备的情况
			auto catalog_node = device_node.child("Catalog");
			if (catalog_node) {
				auto channel_nodes = catalog_node.children("Channel");
				for (auto&& node : channel_nodes) {
					auto channel_info = std::make_shared<ChannelInfo>();
					channel_info->ID = node.child_value("ID");
					channel_info->Name = node.child_value("Name");
					std::string text = node.child_value("URI");
					nbase::StringTrim(text);
					auto pos = text.find_first_of('/');
					if (pos > 0 && pos != std::string::npos) {
						channel_info->App = text.substr(0, pos);
						channel_info->Stream = text.substr(pos + 1);
					}
					device_info->Channels.push_back(channel_info);
				}
				devices.push_back(device_info);
			}
		}
	}
	else {
		LOG(ERROR) << "Devices节点错误";
		return false;
	}
	LOG(INFO) << "配置文件解析完成";
	return true;
}