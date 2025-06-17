#pragma once
#include "Structs.h"

using event_proc = std::function<void(eXosip_event_t*)>;

/// @brief 会话操作，每次会话时生成此对象
class Session :public SessionInfo, public std::enable_shared_from_this<Session>
{
public:
	/// @brief 开始播放。接收到ACK消息后，让流媒体服务器向指定地址发送视频数据。
	void Start();

	void LoadMP4File();

	/// @brief 停止播放，通过app和stream来区分是哪个流。
	void Stop();

	/// @brief 是否暂停，只针对数据回放
	/// @param flag 是or否
	void Pause(bool flag);

	/// @brief 跳转到某个位置
	/// @param pos 位置，秒
	void Seek(int64_t pos);

	/// @brief 设置媒体流播放速度，只针对数据回放
	/// @param speed 播放速度
	void Speed(float speed);

private:

	bool _paused = false;
};

/// @brief  模拟一个SIP设备
class SipDevice : public DeviceInfo
{
public:

	/// @brief 构造Device时，需要传递设备信息和服务器信息
	/// @param info 设备信息
	/// @param sip_server_info SIP服务器信息
	SipDevice(std::shared_ptr<DeviceInfo> info, std::shared_ptr<SipServerInfo> sip_server_info);

public:

	/// @brief 设备初始化，创建SIP Context
	/// @return context是否创建成功，正常情况下不会失败
	bool Init();

	/// @brief 启动sip客户端功能
	/// 绑定本地端口，创建线程接收数据，发送注册信息
	/// @return 是否开始，只有端口被占用或内存不足，会返回false
	bool StartSipClient();

	/// @brief 停止sip交互
	void StopSipClient();

	/// @brief 注销
	/// @return 一般都会返回true
	bool Logout();

private:
	/// @brief 轮询接收数据
	void SipRecvEventThread();

	/// @brief 注册失败
	/// @param event  sip事件
	void OnRegistrationFailed(eXosip_event_t* event);

	/// @brief 注册成功，启动心跳线程
	/// @param event  sip事件
	void OnRegistrationSuccess(eXosip_event_t* event);

	/// @brief 收到消息，这里主要处理服务器的查询命令
	/// @param event  sip事件
	void OnMessageNew(eXosip_event_t* event);

	/// @brief 服务端删除设备后，可能会收到此信息，此时重新发送注册信息
	/// @param event  sip事件
	void OnMessageRequestFailed(eXosip_event_t* event);

	/// @brief 开始推流
	/// @param event  sip事件
	void OnCallACK(eXosip_event_t* event);

	/// @brief 停止推流
	/// @param event  sip事件
	void OnCallClosed(eXosip_event_t* event);

	/// @brief Invite之后长时间没有响应，则会触发此事件
	/// @param event 
	void OnCallCancelled(eXosip_event_t* event);

	/// @brief 请求会话，发送sdp信息
	/// @param event  sip事件
	void OnCallInvite(eXosip_event_t* event);

	/// @brief 响应播放控制请求
	/// @param event  sip事件
	void OnCallMessageNew(eXosip_event_t* event);

	/// @brief 服务端订阅消息
	/// @param event  sip事件
	void OnInSubscriptionNew(eXosip_event_t* event);

	/// @brief 当媒体流改变时收到此回调，在这里判断录像回放的媒体流(app为record)是否结束，并发送结束标志到服务端
	/// @param app 媒体流标识一级路径
	/// @param stream 媒体流标识二级路径
	/// @param regist 注册或注销
	void OnStreamChangedCallback(const std::string& app, const std::string& stream, bool regist);


	/// @brief 回复INVITE请求，如果正常(200)时，需要回复SDP内容
	/// @param event 
	/// @param sdp 
	/// @param status 状态值，默认200，其他有错误:400,Not Found:404
	void SendInviteResponse(eXosip_event_t* event, const std::string& sdp, int status = 200);


private:
	/// @brief 返回成功消息
	/// @param event  sip事件
	void SendMesageResponseOK(eXosip_event_t* event);

	/// @brief 响应播放控制命令
	/// @param event sip事件
	void SendCallResponseOK(eXosip_event_t* event);

	/// @brief 心跳任务
	void HeartbeatTask();

	/// @brief 移动设备位置信息订阅测试
	void MobilePositionTask();

	/// @brief 当录像播放完成之后，发送完成标识到服务端
	/// @param session 会话内容
	/// @return 是否发送，这个不重要，一些服务端不会收此消息，而是等待超时之后自动注销
	bool SendStreamFinishedNotify(std::shared_ptr<Session> session);

private:

	/// @brief 处理服务器的查询信息，主要是目录查询、设备信息、和录像内容
	/// @param doc  服务端发送的xml信息
	void OnQueryMessage(pugi::xml_document& doc);


	/// @brief 设备控制请求，如：预置点查询、调用、删除
	/// @param doc 
	void OnDeviceControl(pugi::xml_document& doc);

	/// @brief 生成目录信息
	/// @param sn 消息编号
	/// @return 生成xml
	std::string GenerateCatalogXML(const std::string& sn);

	/// @brief 生成设备信息
	/// @param sn 消息编号
	/// @return 生成xml
	std::string GenerateDeviceInfoXML(const std::string& sn);

	/// @brief 生成视频参数，这里使用固定的内容，有实际场景需要时再修改
	/// @param sn 消息编号
	/// @return 生成xml
	std::string GenerateVideoParamOptXML(const std::string& sn);

	/// @brief 生成基本配置的xml回复，包括服务器信息、域、密码等
	/// @param sn 消息编号
	/// @return 生成xml
	std::string GenerateBasicParamXML(const std::string& sn);


	/// @brief 生成预置点列表xml
	/// @param sn 
	/// @return 
	std::string GeneratePresetListXML(const std::string& sn);

	/// @brief 发送录像文件信息
	/// @param sn 消息编号
	/// @param channel_id 通道ID 
	/// @param start_time 查询的录像开始时间
	/// @param end_time 查询的录像结束时间
	void SendRecordInfo(const std::string& sn, const std::string& channel_id, const std::string& start_time, const std::string& end_time);

private:

	/// @brief 格式化XML内容，一些服务端对于没有被格式化的xml解析失败
	/// @param xml 要被格式化的xml
	/// @return 格式化之后的xml
	std::string FormatXML(const std::string& text);

	/// @brief 根据SDP解析SSRC值，Y字段属于扩展内容
	/// @param text SDP
	/// @return  解析到的ssrc
	std::string ParseSSRC(const std::string& text);



	std::string ParseDownloadSpeed(const std::string& text);

	/// @brief 根据SDP解析要查询的开始时间和结束时间，t字段也属于扩展内容
	/// @param text SDP
	/// @param start_time 录像开始时间
	/// @param end_time 结束时间
	/// @return 是否解析成功
	bool ParseTimeStr(std::string& text, int64_t& start_time, int64_t& end_time);

	/// @brief 发送xml响应消息
	/// @param doc 
	void SendXmlResponse(const pugi::xml_document& doc);

	/// @brief 发送xml响应消息
	/// @param xml 
	void SendXmlResponse(const std::string& xml);

	/// @brief 根据id查询对应的通道内容
	/// @param id 通道ID
	/// @return 查询出的通道对象
	std::shared_ptr<ChannelInfo> FindChannel(const std::string& id);

private:
	eXosip_t* _sip_context = nullptr;
	std::shared_ptr<SipServerInfo> _sip_server_info = nullptr;

private:
	//注册设备的SIP地址
	std::string _from_uri = "";
	//SIP服务器的SIP地址
	std::string _contact_url = "";
	//代理服务器的SIP地址
	std::string _proxy_uri = "";

	//服务器域名
	std::string _sip_server_domain = "";

	//注册ID，使用 eXosip_register_build_initial_register 创建时生成
	int _register_id = -1;

	//是否正在运行
	bool _is_running = false;

	//心跳线程运行
	bool _is_heartbeat_running = false;

	//是否注册成功
	bool _register_success = false;

	//订阅会话ID
	int32_t _subscription_dialog_id = -1;

	//控制订阅消息定时发送
	std::mutex _subscription_mutex;
	std::condition_variable _subscription_condition;

	//控制定时发送心跳信息
	std::mutex _heartbeat_mutex;
	std::condition_variable _heartbeat_condition;

	//会话信息，DialogID和Session关联
	std::mutex _session_mutex;
	std::unordered_map<int32_t, std::shared_ptr<Session>> _session_map;

	//消息和函数映射
	std::unordered_map<eXosip_event_type_t, event_proc> _event_processor_map;

private:
	//SIP消息线程
	std::shared_ptr<std::thread> _sip_thread = nullptr;
	//心跳线程
	std::shared_ptr<std::thread> _heartbeat_thread = nullptr;
	//订阅消息回复线程
	std::shared_ptr<std::thread> _subscription_thread = nullptr;
};





