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
        server_info->IP = "0.0.0.0";
        // SIP 服务器固定端口
        nbase::StringToInt(sip_server_node.child_value("Port"), &server_info->Port);
        // SIP 服务器ID
        server_info->ID = sip_server_node.child_value("ID");
        server_info->Domain = sip_server_node.child_value("Domain");
        server_info->Nonce = GenerateRandomString(16);
        // SIP 服务器固定密码
        server_info->Password = sip_server_node.child_value("Password");
    } else {
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
    } else {
        LOG(ERROR) << "MediaServer节点错误";
        return false;
    }

    LOG(INFO) << "配置文件解析完成";
    return true;
}