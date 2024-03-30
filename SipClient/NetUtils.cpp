#include "pch.h"
#include "NetUtils.h"

#ifdef _WIN32

#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")


std::vector<uint16_t> NetHelper::GetAllTcpConnectionsPort() {
	std::vector<uint16_t> ret;
	ULONG size = 0;
	GetTcpTable(NULL, &size, TRUE);
	std::unique_ptr<char[]> buffer(new char[size]);

	PMIB_TCPTABLE tcpTable = reinterpret_cast<PMIB_TCPTABLE>(buffer.get());
	if (GetTcpTable(tcpTable, &size, FALSE) == NO_ERROR) {
		for (size_t i = 0; i < tcpTable->dwNumEntries; i++) {
			ret.push_back(ntohs((uint16_t)tcpTable->table[i].dwLocalPort));
		}
	}
	std::sort(std::begin(ret), std::end(ret));
	return ret;
}


std::vector<uint16_t> NetHelper::GetAllUdpConnectionsPort() {
	std::vector<uint16_t> ret;
	ULONG size = 0;
	GetUdpTable(NULL, &size, TRUE);
	std::unique_ptr<char[]> buffer(new char[size]);

	PMIB_UDPTABLE udpTable = reinterpret_cast<PMIB_UDPTABLE>(buffer.get());
	if (GetUdpTable(udpTable, &size, FALSE) == NO_ERROR) {
		for (size_t i = 0; i < udpTable->dwNumEntries; i++) {
			ret.push_back(ntohs((uint16_t)udpTable->table[i].dwLocalPort));
		}
	}
	std::sort(std::begin(ret), std::end(ret));
	return ret;
}


uint16_t NetHelper::FindAvailableTcpPort(uint16_t begin, uint16_t end) {
	auto vec = GetAllTcpConnectionsPort();
	for (uint16_t port = begin; port != end; ++port) {
		if (!std::binary_search(std::begin(vec), std::end(vec), port)) {
			return port;
		}
	}
	return 0;
}


uint16_t NetHelper::FindAvailableUdpPort(uint16_t begin, uint16_t end) {
	auto vec = GetAllUdpConnectionsPort();
	for (uint16_t port = begin; port != end; ++port) {
		if (!std::binary_search(std::begin(vec), std::end(vec), port)) {
			return port;
		}
	}
	return 0;
}


uint16_t NetHelper::FindAvailablePort(uint16_t begin, uint16_t end) {
	auto vecTcp = GetAllTcpConnectionsPort();
	auto vecUdp = GetAllUdpConnectionsPort();
	for (uint16_t port = begin; port != end; ++port) {
		if (!std::binary_search(std::begin(vecTcp), std::end(vecTcp), port)
			&& !std::binary_search(std::begin(vecUdp), std::end(vecUdp), port)) {
			return port;
		}
	}
	return 0;
}


bool NetHelper::IsPortAvailable(uint16_t port) {
	auto vecTcp = GetAllTcpConnectionsPort();
	auto vecUdp = GetAllUdpConnectionsPort();
	if (!std::binary_search(std::begin(vecTcp), std::end(vecTcp), port)
		&& !std::binary_search(std::begin(vecUdp), std::end(vecUdp), port)) {
		return true;
	}
	return false;
}


#endif