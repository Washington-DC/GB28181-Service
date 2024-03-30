/*****************************************************************//**
 * \file   NetUtils.h
 * \brief  windows系统下，获取可用端口信息
 * 
 * \author yszs
 * \date   March 2024
 *********************************************************************/
#pragma once

#ifdef _WIN32

#define PORT_MIN 30000
#define PORT_MAX 65535

namespace NetHelper {

	/// @brief 获取所有TCP已使用的端口
	/// @return 所有TCP已使用的端口
	std::vector<uint16_t> GetAllTcpConnectionsPort();

	/// @brief 获取所有UDP已使用的端口
	/// @return 所有UDP已使用的端口
	std::vector<uint16_t> GetAllUdpConnectionsPort();

	/// @brief 获取一个可用的TCP端口
	/// @param begin 端口开始范围
	/// @param end 端口结束范围
	/// @return 可用的TCP端口
	uint16_t FindAvailableTcpPort(uint16_t begin = PORT_MIN, uint16_t end = PORT_MAX);

	/// @brief 获取一个可用的UDP端口
	/// @param begin 端口开始范围
	/// @param end 端口结束范围
	/// @return 可用的UDP端口
	uint16_t FindAvailableUdpPort(uint16_t begin = PORT_MIN, uint16_t end = PORT_MAX);

	/// @brief 获取一个可用的端口，不区分TCP/UDP
	/// @param begin 端口开始范围
	/// @param end 端口结束范围
	/// @return 可用的端口
	uint16_t FindAvailablePort(uint16_t begin = PORT_MIN, uint16_t end = PORT_MAX);

	/// @brief 判断端口是否被占用
	/// @param port 端口
	/// @return 是否被占用
	bool IsPortAvailable(uint16_t port);
}; // namespace NetHelper

#endif