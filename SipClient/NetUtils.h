#pragma once

#define PORT_MIN 30000
#define PORT_MAX 65535

#ifdef _WIN32

namespace NetHelper {
std::vector<uint16_t> GetAllTcpConnectionsPort();
std::vector<uint16_t> GetAllUdpConnectionsPort();
uint16_t FindAvailableTcpPort(uint16_t begin = PORT_MIN, uint16_t end = PORT_MAX);
uint16_t FindAvailableUdpPort(uint16_t begin = PORT_MIN, uint16_t end = PORT_MAX);
uint16_t FindAvailablePort(uint16_t begin = PORT_MIN, uint16_t end = PORT_MAX);
bool IsPortAvailable(uint16_t port);
}; // namespace NetHelper

#endif