cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread -std=c++17  -Wno-error=deprecated-declarations -Wno-deprecated-declarations")
set(CFLAGS "-march=armv8-a -mthumb -mfpu=neon-vfpv4")

add_definitions(-D__ARMEL__)   # 在全局范围内定义

include_directories(..)
include_directories(.)



file(GLOB MY_SOURCES "build/*.cpp" "encrypt/*.cpp" "file/*.cpp" "framework/*.cpp" "hardware/*.cpp" "memory/*.cpp"
 "network/*.cpp" "synchronization/*.cpp" "thread/*.cpp" "time/*.cpp" "util/*.cpp" "win32/*.cpp" "*.cpp" "*.h" 
  "third_party/chrome/*.cpp"  "third_party/convert_utf/*.cpp"  "third_party/modp_b64/*.cpp" )

add_library(base  ${MY_SOURCES})




