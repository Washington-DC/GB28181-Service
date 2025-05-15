// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#define WIN32_LEAN_AND_MEAN

#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <csignal>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <base/base.h>
#include <eXosip2/eXosip.h>
#include <spdlog/spdlog.h>
#if defined(_WIN32) && defined(_MSC_VER)
#include <spdlog/sinks/msvc_sink.h>
#endif
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/chrono.h>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>

#include <Poller/EventPoller.h>
#include <Poller/Timer.h>
#include <Util/File.h>

using namespace std::string_literals;
using namespace std::chrono_literals;

namespace fs = std::filesystem;

#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Dnsapi.lib")

#pragma comment(lib, "osipparser2.lib")
#pragma comment(lib, "osip2.lib")
#pragma comment(lib, "eXosip.lib")
#pragma comment(lib, "libcares.lib")

#pragma comment(lib, "sqlite3.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "ZLToolKit.lib")

#ifdef _DEBUG
#pragma comment(lib, "pugixmld.lib")
#pragma comment(lib, "libcurl-d_imp.lib")
#pragma comment(lib, "cpr-d.lib")
#else
#pragma comment(lib, "pugixml.lib")
#pragma comment(lib, "libcurl_imp.lib")
#pragma comment(lib, "cpr.lib")
#endif

#endif 
#endif // PCH_H
