// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。

std::string GenerateRandomString(int n)
{
	const std::string chars("0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	int i = 0;
	int len = chars.size();
	std::string text;
	while (i < n)
	{
		int idx = rand() % len;
		text.push_back(chars[idx]);
		++i;
	}

	return text;
}
