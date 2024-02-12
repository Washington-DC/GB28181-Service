cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread -std=c++17  -Wno-error=deprecated-declarations -Wno-deprecated-declarations")

add_definitions(-DENABLE_MAIN_SOCKET)
# add_definitions(-DSPDLOG_FMT_EXTERNAL)

aux_source_directory(. files)
add_executable(SipService ${files} ${sqlite})

target_link_libraries(SipService base)
target_link_libraries(SipService zltoolkit)
# target_link_libraries(SipService fmt)
# target_link_libraries(SipService spdlog)
target_link_libraries(SipService pugixml)
target_link_libraries(SipService cpr)
target_link_libraries(SipService ${Boost_LIBRARIES})
target_link_libraries(SipService PkgConfig::sqlite3)
target_link_libraries(SipService PkgConfig::osip2)
target_link_libraries(SipService PkgConfig::exosip2)




