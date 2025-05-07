/*****************************************************************//**
 * \file   HttpServer.h
 * \brief  建立一个HTTP服务，用户接收流媒体服务器的媒体流活动信息
 *
 * \author yszs
 * \date   March 2024
 *********************************************************************/
#pragma once
#include <crow_all.h>

namespace dto
{
	//媒体流信息
	struct ZlmStreamInfo
	{
		std::string MediaServerID;//服务器ID
		std::string App;	//媒体流路径
		std::string Stream; //媒体流路径
		std::string Schema; //协议类型
		std::string Vhost;  //虚拟主机
		int	OriginType;		//源类型
		bool Regist;		//注册or注销
		uint16_t Port;		//数据端口
		std::string IP;		//媒体流ID
		std::string Params;	//参数
		std::string Path() { return App + "/" + Stream; }
	};


	struct ZlmMP4Item
	{
		std::string MediaServerID;//服务器ID
		std::string App;	//媒体流路径
		std::string Stream; //媒体流路径
		std::string FileName; //文件名
		std::string FilePath; //文件绝对路径
		int64_t FileSize;  //文件大小，单位字节
		std::string Folder;  //文件所在目录路径
		int64_t StartTime;  //开始录制时间戳
		int64_t TimeDuration;  //录制时长，单位秒
		std::string URL;  //http/rtsp/rtmp点播相对url路径
		std::string Path() { return App + "/" + Stream; }
		std::string DbName() { return App + "_" + Stream; }
	};

	/// @brief 将json信息反序列化到实际对象
	/// @param j json内容
	/// @param info 输出的实例
	void from_json(const nlohmann::json& j, ZlmStreamInfo& info);

	/// @brief 将实例对象序列化为json，用不到，但是要有这个接口
	/// @param j 
	/// @param p 
	void to_json(nlohmann::json& j, const ZlmStreamInfo& p);


	/// @brief 将json信息反序列化到实际对象
	/// @param j json内容
	/// @param info 输出的实例
	void from_json(const nlohmann::json& j, ZlmMP4Item& info);

	/// @brief 将实例对象序列化为json，用不到，但是要有这个接口
	/// @param j 
	/// @param p 
	void to_json(nlohmann::json& j, const ZlmMP4Item& p);
}

class HttpServer
{
	using OnStreamChangedCallback = std::function<void(const std::string& app, const std::string& stream, bool regist)>;
public:
	SINGLETON_DEFINE(HttpServer);

	/// @brief 启动http服务
	/// @param port http端口
	/// @return 返回std::future<void>，否则将会阻塞在这里
	std::future<void> Start(int port);

	/// @brief 添加接收媒体流改变的回调消息
	/// @param func 
	void AddStreamChangedCallback(OnStreamChangedCallback func);

private:

	/// @brief 在构造函数中，添加http接口和对应的事件响应
	HttpServer();

	//接收媒体流改变的回调
	std::vector<OnStreamChangedCallback> _vec_stream_changed_callback;
	std::mutex _mtx;


	crow::SimpleApp _app;

	//这里创建一个blueprint，用户接收hook消息
	crow::Blueprint _hook_blueprint;
};