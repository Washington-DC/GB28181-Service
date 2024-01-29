#include "pch.h"
#include "Utils.h"
#include <fmt/chrono.h>

std::string LocalTime(time_t time)
{
	return fmt::format("{:%Y-%m-%d %H:%M:%S}", fmt::localtime(time));
}

std::string GenerateRandomString(int n)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 1000);

	const std::string chars("0123456789"
							"abcdefghijklmnopqrstuvwxyz"
							"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	int i = 0;
	int len = (int)chars.size();
	std::string text;
	while (i < n)
	{
		int idx = dis(gen) % len;
		text.push_back(chars[idx]);
		++i;
	}

	return text;
}

std::string SSRC_Hex(std::string ssrc)
{
	return fmt::format("{:08X}", std::stol(ssrc));
}

std::string ToUtf8String(const std::string &input)
{
#ifdef _WIN32
	return nbase::win32::MBCSToUtf8(input);
#else
	return input;
#endif
}

std::string ToMbcsString(const std::string &input)
{
#ifdef _WIN32
	return nbase::win32::Utf8ToMBCS(input);
#else
	return input;
#endif
}

std::string GetCurrentModuleDirectory()
{
#ifdef _WIN32
	return nbase::win32::GetCurrentModuleDirectoryA();
#else
	char buffer[1024] = {};
	readlink("/proc/self/exe", buffer, sizeof(buffer));
	return fs::path(buffer).parent_path().string();
#endif
}