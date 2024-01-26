#pragma once
#include "SSRC_Config.h"

enum STREAM_TYPE {
	STREAM_TYPE_NONE = 0,
	STREAM_TYPE_GB = 1,
	STREAM_TYPE_PROXY = 2,
	STREAM_TYPE_PUSH = 3,
	STREAM_TYPE_MAX
};

class MediaStream
{
public:
	typedef std::shared_ptr<MediaStream> Ptr;
	MediaStream() = default;
	MediaStream(const std::string& app, const std::string& stream_id, STREAM_TYPE type);

	//MARK: 基类必须要有一个虚函数
	virtual ~MediaStream() {}

	std::string GetApp() const;
	std::string GetStreamID() const;
	STREAM_TYPE GetType();

	nlohmann::json toJson();

	eXosip_t* exosip_context = nullptr;

private:
	std::string _app;
	std::string _stream_id;
	STREAM_TYPE _type = STREAM_TYPE::STREAM_TYPE_NONE;
};


class MediaStreamProxy :public MediaStream
{
public:
	MediaStreamProxy(const std::string& app, const std::string& stream_id)
		:MediaStream(app, stream_id, STREAM_TYPE::STREAM_TYPE_PROXY) {};
};

class MediaStreamPushed :public MediaStream
{
public:
	MediaStreamPushed(const std::string& app, const std::string& stream_id)
		:MediaStream(app, stream_id, STREAM_TYPE::STREAM_TYPE_PUSH) {};
};


class CallSession :public MediaStream
{
public:
	typedef std::shared_ptr<CallSession> Ptr;
	CallSession(const std::string& app, const std::string& stream_id,const SSRCInfo::Ptr ssrc)
		:MediaStream(app, stream_id, STREAM_TYPE::STREAM_TYPE_GB)
		,_ssrc(ssrc) {};

	virtual ~CallSession() {}

	int GetCallID();
	void SetCallID(int id);
	int GetDialogID();
	void SetDialogID(int id);
	SSRCInfo::Ptr GetSSRCInfo();
	void SetSSRCInfo(SSRCInfo::Ptr ssrc);
	void SetConnected(bool flag);
	bool IsConnected();

	bool WaitForStreamReady(int seconds = 6);
	void NotifyStreamReady();

private:
	SSRCInfo::Ptr _ssrc;
	bool _is_connected = false;
	int _call_id = 0;
	int _dialog_id = 0;

	std::mutex _mutex;
	std::condition_variable _cv;

};

class StreamManager
{
public:
	SINGLETON_DEFINE(StreamManager);

	void AddStream(MediaStream::Ptr stream);
	void RemoveStream(const std::string& id);

	MediaStream::Ptr GetStream(const std::string& id);
	MediaStream::Ptr GetStreamByCallID(int id);
	std::vector<MediaStream::Ptr> GetAllStream();
	std::vector<MediaStream::Ptr> GetStreamByType(STREAM_TYPE type);
	MediaStream::Ptr MakeStream(const std::string& stream_id, const std::string& app, STREAM_TYPE type);
	void ClearStreams();

private:
	StreamManager() = default;

private:
	std::mutex _mutex;
	std::map<std::string, MediaStream::Ptr> _streams;
};