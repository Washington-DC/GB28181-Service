#include "pch.h"
#include "ConfigManager.h"
#include "Utils.h"

bool ConfigManager::LoadConfig(std::string filepath)
{
	SPDLOG_INFO("配置文件解析...");
	pugi::xml_document doc;
	auto ret = doc.load_file(filepath.c_str(), pugi::parse_full);
	if (ret.status != pugi::status_ok)
	{
		SPDLOG_ERROR("配置文件解析失败");
		return false;
	}

	auto root = doc.child("Config");
	// SIP 服务器
	auto sip_server_node = root.child("SipServer");
	if (sip_server_node)
	{
		server_info = std::make_shared<SipServerInfo>();
		// SIP 服务器IP
		server_info->IP = sip_server_node.child_value("IP");
		// SIP 服务器固定端口
		server_info->Port = sip_server_node.child("Port").text().as_int();
		// SIP 服务器ID
		server_info->ID = sip_server_node.child_value("ID");
		// 取ID的前10位
		server_info->Realm = server_info->ID.substr(0, 10);
		// 每次运行的时候随机生成
		server_info->Nonce = GenerateRandomString(16);
		//SPDLOG_INFO( server_info->Nonce;
		// SIP 服务器固定密码
		server_info->Password = sip_server_node.child_value("Password");
		//流媒体服务器公网IP
		server_info->ExternIP = sip_server_node.child_value("ExternIP");
	}
	else
	{
		SPDLOG_ERROR("SipServer节点错误");
		return false;
	}

	//流媒体服务
	auto media_server_node = root.child("MediaServer");
	if (media_server_node)
	{
		media_server_info = std::make_shared<MediaServerInfo>();

		//流媒体服务 IP
		media_server_info->IP = media_server_node.child_value("IP");
		//流媒体服务 端口
		media_server_info->Port = media_server_node.child("Port").text().as_int();

		media_server_info->PlayWait = media_server_node.child("PlayWait").text().as_int(15);
		//流媒体服务 如果不是127.0.0.1的话，需要校验Secret字段
		media_server_info->Secret = media_server_node.child_value("Secret");

		media_server_info->SinglePortMode = media_server_node.child("SinglePortMode").text().as_bool();

		media_server_info->RtpPort = media_server_node.child("RtpPort").text().as_int();
	}
	else
	{
		SPDLOG_ERROR("MediaServer节点错误");
		return false;
	}


	//本地Http服务
	auto http_node = root.child("Http");
	if (http_node)
	{
		//流媒体服务 端口
		http_port = http_node.child("Port").text().as_int();
	}
	else
	{
		SPDLOG_ERROR("Http节点错误");
		return false;
	}

	SPDLOG_INFO("配置文件解析完成");
	return true;
}