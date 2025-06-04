cmake_minimum_required(VERSION 3.5.0)
project(SipService VERSION 1.0.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17 -Wl,-rpath=./:./lib")

set(CMAKE_CXX_STANDARD 17)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/release/linux/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

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


if(UNIX)
  # UNIX/Linux/Darwin
  set(COMPILE_OPTIONS_DEFAULT
    "-fPIC"
    "-Wall;-Wextra"
    "-Wno-unused-function;-Wno-unused-parameter;-Wno-unused-variable;-Wno-unused-local-typedefs"
    "-Wno-error=extra;-Wno-error=missing-field-initializers;-Wno-error=type-limits")
elseif(WIN32)
  set(COMPILE_OPTIONS_DEFAULT
    "/wd4566;/wd4819"
    "/EHsc")
endif()

include_directories(${CMAKE_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/submodule/ZLToolKit/src)
include_directories(${CMAKE_SOURCE_DIR}/submodule/crow)
include_directories(${CMAKE_SOURCE_DIR}/submodule/json/single_include)
include_directories(${CMAKE_SOURCE_DIR}/submodule/magic_enum/include)
include_directories(${CMAKE_SOURCE_DIR}/submodule/sqlite3pp/headeronly_src)
include_directories(${CMAKE_SOURCE_DIR}/submodule/spdlog/include)
include_directories(${CMAKE_SOURCE_DIR}/submodule/pugixml/src)
include_directories(${CMAKE_SOURCE_DIR}/submodule/cpr/include)

include_directories(${CMAKE_SOURCE_DIR}/submodule/osip/include)
include_directories(${CMAKE_SOURCE_DIR}/submodule/exosip/include)

find_program(MAKE_EXE NAMES gmake nmake make)

message(STATUS "===========CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

include(ExternalProject)
# 定义子模块的构建目录和安装前缀
set(OSIP_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/osip_install")
set(EXOSIP_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/exosip_install")


# 查找 ExternalProject 安装的 exosip
find_library(OSIP2_LIB NAMES osip2
            PATHS ${OSIP_INSTALL_PREFIX}/lib
           NO_DEFAULT_PATH )
find_library(OSIP_PARSER2_LIB NAMES osipparser2
            PATHS ${OSIP_INSTALL_PREFIX}/lib
            NO_DEFAULT_PATH)
find_path(OSIP2_HEADER_DIR osip.h
          PATHS ${OSIP_INSTALL_PREFIX}/include/osip2
          NO_DEFAULT_PATH)

# 查找 ExternalProject 安装的 exosip
find_library(EXOSIP2_LIB NAMES eXosip2
            PATHS ${EXOSIP_INSTALL_PREFIX}/lib
)
find_path(EXOSIP2_HEADER_DIR eXosip.h
          PATHS ${EXOSIP_INSTALL_PREFIX}/include/eXosip2
         NO_DEFAULT_PATH )


if(OSIP2_LIB AND OSIP2_HEADER_DIR)
    set(OSIP2_FOUND TRUE)
    set(OSIP2_LIBRARIES ${OSIP2_LIB})
    set(OSIP_PARSER2_LIBRARIES ${OSIP_PARSER2_LIB})
    get_filename_component(OSIP2_INCLUDE_DIR ${OSIP2_HEADER_DIR} DIRECTORY)
    set(OSIP2_INCLUDE_DIRS ${OSIP2_INCLUDE_DIR})
else()
    message(STATUS "Could not find osip2 libraries or headers installed by ExternalProject")
endif()


if(EXOSIP2_LIB AND EXOSIP2_HEADER_DIR)
    set(EXOSIP2_FOUND TRUE)
    set(EXOSIP2_LIBRARIES ${EXOSIP2_LIB})

    get_filename_component(EXOSIP2_INCLUDE_DIR ${EXOSIP2_HEADER_DIR} DIRECTORY)
    set(EXOSIP2_INCLUDE_DIRS ${EXOSIP2_INCLUDE_DIR})
else()
    message(STATUS "Could not find exosip2 libraries or headers installed by ExternalProject")
endif()


find_package(PkgConfig REQUIRED)
pkg_search_module(sqlite3 REQUIRED sqlite3 IMPORTED_TARGET)


find_package(Boost REQUIRED COMPONENTS thread locale system filesystem)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)


add_subdirectory(submodule)

add_subdirectory(base)
# add_subdirectory(SipService)
add_subdirectory(SipClient)
