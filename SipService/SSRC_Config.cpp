#include "pch.h"
#include "SSRC_Config.h"

SSRCInfo::SSRCInfo(int port, const std::string& ssrc, const std::string& stream_id)
	:_port(port)
	, _ssrc(ssrc)
	, _stream_id(stream_id)
{

}

int SSRCInfo::GetPort()
{
	return _port;
}

void SSRCInfo::SetPort(int port)
{
	_port = port;
}

std::string SSRCInfo::GetSSRC() const
{
	return _ssrc;
}

void SSRCInfo::SetSSRC(const std::string& ssrc)
{
	_ssrc = ssrc;
}

std::string SSRCInfo::GetStreamID() const
{
	return _stream_id;
}

void SSRCInfo::SetStreamID(const std::string& id)
{
	_stream_id = id;
}

void SSRCConfig::SetPrefix(const std::string& pre)
{
	_prefix = pre;
}

std::string SSRCConfig::GenerateSSRC(SSRCConfig::Mode mode)
{
	//std::random_device rd;
	//std::mt19937 gen(rd());
	//std::uniform_int_distribution<> dis(1, 9999);
	//return fmt::format("{:01d}{}{:04d}", mode, _prefix, dis(gen));
	return fmt::format("{:01d}{}{:04d}", (int)mode, _prefix, _ssrc_idx++);
}