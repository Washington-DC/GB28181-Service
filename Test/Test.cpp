// Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <bitset>
#include <iomanip>

int main()
{
	std::string hexStr = "A50F0102000100B8"; // 16进制字符串
	std::vector<unsigned char> byteArr; // 存储byte数组的vector

	// 将16进制字符串转为byte数组
	for (size_t i = 0; i < hexStr.length(); i += 2) {
		std::string byteString = hexStr.substr(i, 2);
		unsigned char byte = (unsigned char)stoul(byteString, nullptr, 16);
		byteArr.push_back(byte);
	}

	std::bitset<8> cmd(byteArr[3]);

	std::cout <<  cmd <<std::endl;

	if (cmd.test(0)) std::cout << "右移";
	if (cmd.test(1)) std::cout << "左移";
	if (cmd.test(2)) std::cout << "下移";
	if (cmd.test(3)) std::cout << "上移";
	if (cmd.test(4)) std::cout << "放大";
	if (cmd.test(5)) std::cout << "缩小";

	std::cout << std::endl;

	std::cout << "水平速度:" << (int)byteArr[4] << std::endl;
	std::cout << "垂直速度:" << (int)byteArr[5] << std::endl;
	std::cout << "变倍速度:" << (int)(byteArr[6] >> 4) << std::endl;

	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
