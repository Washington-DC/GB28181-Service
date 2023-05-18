#pragma once

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
    MediaStream();


private:
	std::string _app;
	std::string _stream_id;
    STREAM_TYPE _type = STREAM_TYPE::STREAM_TYPE_NONE;
};

class StreamManager
{
public:

};