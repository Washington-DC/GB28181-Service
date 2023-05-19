#pragma once

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



class SSRCConfig
{
	enum class Mode:int
	{
		Realtime,
		Playback
	};
public:
	typedef std::shared_ptr<SSRCConfig> Ptr;

	SSRCConfig() = default;

	void SetPrefix(const std::string& pre);
	std::string GenerateSSRC(Mode m = Mode::Realtime);

private:
	std::string _prefix = "";

};