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

std::string ToUtf8String(const std::string& input)
{
#ifdef _WIN32
	return nbase::win32::MBCSToUtf8(input);
#else
	return input;
#endif
}

std::string ToMbcsString(const std::string& input)
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


//格式化后的时间字符串转化为std::time_t格式，所有的输出的时间戳都需要转化为GMT时间
int64_t ISO8601ToTimeT(const std::string& str)
{
	std::regex pattern(R"(^\d{4}-\d{2}-\d{2}[Tt]\d{2}:\d{2}:\d{2}[Zz]?$)");
	if (std::regex_match(str,  pattern)) {
		std::tm t;
		auto year = str.substr(0, 4);
		t.tm_year = std::stoi(year) - 1900;
		auto month = str.substr(5, 2);
		t.tm_mon = std::stoi(month) - 1;
		auto day = str.substr(8, 2);
		t.tm_mday = std::stoi(day);
		auto hour = str.substr(11, 2);
		t.tm_hour = std::stoi(hour);
		auto minute = str.substr(14, 2);
		t.tm_min = std::stoi(minute);
		auto second = str.substr(17, 2);
		t.tm_sec = std::stoi(second);

		// GMT，不以z结尾为本地时间,本地时间转换为标准时间
		if (str.back() != 'z' && str.back() != 'Z')
		{
			auto tt = std::mktime(&t);
			auto tm = std::localtime(&tt);
			return std::mktime(tm);
		}
		else// 即是标准时间
		{
			return std::mktime(&t);
		}
	}
	else {
		LOG(ERROR) << "时间格式校验错误: " << str;
		return 0;
	}
}
