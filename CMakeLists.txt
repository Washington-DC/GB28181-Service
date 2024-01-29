cmake_minimum_required(VERSION 3.5.0)
project(SipService VERSION 1.0.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17 -Wl,-rpath=./:./lib")

set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release/linux/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
# Debug 配置
message(STATUS "CMAKE_BUILD_TYPE Debug")

elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
message(STATUS "CMAKE_BUILD_TYPE Release")
# Release 配置
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -Wall -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -s")
endif()

# 方便修改全局变量
function(update_cached name value)
  set("${name}" "${value}" CACHE INTERNAL "*** Internal ***" FORCE)
endfunction()

function(update_cached_list name)
  set(_tmp_list "${${name}}")
  list(APPEND _tmp_list "${ARGN}")
  list(REMOVE_DUPLICATES _tmp_list)
  update_cached(${name} "${_tmp_list}")
endfunction()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ZLToolKit/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/depends/crow)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/depends/json)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/depends/magic_enum)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/depends/sqlite/include)

find_package(PkgConfig REQUIRED)
pkg_search_module(osip2 REQUIRED libosip2 IMPORTED_TARGET)
pkg_search_module(exosip2 REQUIRED libexosip2 IMPORTED_TARGET)
pkg_search_module(sqlite3 REQUIRED sqlite3 IMPORTED_TARGET)


find_package(glog  REQUIRED)
find_package(fmt  REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread locale system filesystem)
find_package(pugixml  REQUIRED)
find_package(cpr REQUIRED)


add_subdirectory(3rdparty)
add_subdirectory(base)
add_subdirectory(SipService)
add_subdirectory(SipClient)
