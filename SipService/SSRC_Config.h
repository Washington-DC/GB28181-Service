#pragma once

//SSRC信息，包括SSRC字段、数据通信端口等缓存内容
class SSRCInfo
{
public:
	typedef std::shared_ptr<SSRCInfo> Ptr;

	SSRCInfo(int port,const std::string& ssrc,const std::string& stream_id);

	int GetPort();
	void SetPort(int port);
	std::string GetSSRC() const;
	void SetSSRC(const std::string& ssrc);
	std::string GetStreamID() const;
	void SetStreamID(const std::string& id);

private:
	int _port = 0;
	std::string _ssrc;
	std::string _stream_id;
};


//SSRC配置，设置前缀，生成
class SSRCConfig
{
public:
	enum class Mode :int
	{
		Realtime,
		Playback
	};

	typedef std::shared_ptr<SSRCConfig> Ptr;
	SINGLETON_DEFINE(SSRCConfig);

	void SetPrefix(const std::string& pre);
	std::string GenerateSSRC(Mode m = Mode::Realtime);

private:
	SSRCConfig() = default;
	std::string _prefix = "";

	std::atomic_uint16_t _ssrc_idx = {0};
};