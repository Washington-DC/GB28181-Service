cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread -std=c++17  -Wno-error=deprecated-declarations -Wno-deprecated-declarations")

add_definitions(-DENABLE_MAIN_SOCKET)

aux_source_directory(. files)
add_executable(SipClient ${files})

target_link_libraries(SipClient base)
target_link_libraries(SipClient zltoolkit)
target_link_libraries(SipClient fmt)
target_link_libraries(SipClient glog)
target_link_libraries(SipClient pugixml)
target_link_libraries(SipClient cpr)
target_link_libraries(SipClient ${Boost_LIBRARIES})
target_link_libraries(SipClient PkgConfig::osip2)
target_link_libraries(SipClient PkgConfig::exosip2)




