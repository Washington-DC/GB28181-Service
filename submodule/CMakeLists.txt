# toolkit
# TODO: 改造 toolkit 以便直接引用

include(CheckStructHasMember)
include(CheckSymbolExists)

# 检查 sendmmsg 相关依赖并设置对应的宏, 配置 _GNU_SOURCE 以启用 GNU 扩展特性
list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_struct_has_member("struct mmsghdr" msg_hdr sys/socket.h HAVE_MMSG_HDR)
check_symbol_exists(sendmmsg sys/socket.h HAVE_SENDMMSG_API)
check_symbol_exists(recvmmsg sys/socket.h HAVE_RECVMMSG_API)

set(COMPILE_DEFINITIONS)
# ToolKit 依赖 ENABLE_OPENSSL 以及 ENABLE_MYSQL
list(FIND MK_COMPILE_DEFINITIONS ENABLE_OPENSSL ENABLE_OPENSSL_INDEX)
if(NOT ENABLE_OPENSSL_INDEX EQUAL -1)
  list(APPEND COMPILE_DEFINITIONS ENABLE_OPENSSL)
endif()
list(FIND MK_COMPILE_DEFINITIONS ENABLE_MYSQL   ENABLE_MYSQL_INDEX)
if(NOT ENABLE_MYSQL_INDEX EQUAL -1)
  list(APPEND COMPILE_DEFINITIONS ENABLE_MYSQL)
endif()
if(HAVE_MMSG_HDR)
  list(APPEND COMPILE_DEFINITIONS HAVE_MMSG_HDR)
endif()
if(HAVE_SENDMMSG_API)
  list(APPEND COMPILE_DEFINITIONS HAVE_SENDMMSG_API)
endif()
if(HAVE_RECVMMSG_API)
  list(APPEND COMPILE_DEFINITIONS HAVE_RECVMMSG_API)
endif()

set(ToolKit_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ZLToolKit)
# 收集源代码
file(GLOB ToolKit_SRC_LIST
  ${ToolKit_ROOT}/src/*/*.cpp
  ${ToolKit_ROOT}/src/*/*.h
  ${ToolKit_ROOT}/src/*/*.c)
if(IOS)
  list(APPEND ToolKit_SRC_LIST
    ${ToolKit_ROOT}/Network/Socket_ios.mm)
endif()

# 去除 win32 的适配代码
if(NOT WIN32)
  list(REMOVE_ITEM ToolKit_SRC_LIST ${ToolKit_ROOT}/win32/getopt.c)
else()
  # 防止 Windows.h 包含 Winsock.h
  list(APPEND COMPILE_DEFINITIONS
    WIN32_LEAN_AND_MEAN MP4V2_NO_STDINT_DEFS
    # 禁用警告
    _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

# 添加库
add_library(zltoolkit STATIC ${ToolKit_SRC_LIST})
add_library(ZLMediaKit::ToolKit ALIAS zltoolkit)
target_compile_definitions(zltoolkit
  PUBLIC ${COMPILE_DEFINITIONS})
target_compile_options(zltoolkit
  PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_include_directories(zltoolkit
  PRIVATE
    "$<BUILD_INTERFACE:${ToolKit_ROOT}/src>"
  PUBLIC
    "$<BUILD_INTERFACE:${ToolKit_ROOT}>/src")


update_cached_list(MK_LINK_LIBRARIES ZLMediaKit::ToolKit)

if(USE_SOLUTION_FOLDERS AND (NOT GROUP_BY_EXPLORER))
    # 在 IDE 中对文件进行分组, 源文件和头文件分开
    set_file_group(${ToolKit_ROOT}/src ${ToolKit_SRC_LIST})
endif()

add_subdirectory("pugixml")

set(SPDLOG_BUILD_TESTING VALUE OFF CACHE BOOL "Build spdlog tests" FORCE)
add_subdirectory("spdlog")

add_subdirectory("json")

set(CPR_USE_SYSTEM_CURL VALUE ON CACHE BOOL  "If enabled we will use the curl lib already installed on this system." FORCE)
add_subdirectory("cpr")

add_subdirectory("magic_enum")


