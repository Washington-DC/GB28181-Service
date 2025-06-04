cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread -std=c++17  -Wno-error=deprecated-declarations -Wno-deprecated-declarations")

add_definitions(-DENABLE_MAIN_SOCKET)

aux_source_directory(. files)
add_executable(SipClient ${files})

target_link_libraries(SipClient base)
target_link_libraries(SipClient zltoolkit)
target_link_libraries(SipClient pugixml)
target_link_libraries(SipClient cpr)
target_link_libraries(SipClient ${Boost_LIBRARIES})
target_link_libraries(SipClient PkgConfig::sqlite3)
# target_link_libraries(SipClient PkgConfig::osip2)
# target_link_libraries(SipClient PkgConfig::exosip2)
target_link_libraries(SipClient ${OPENSSL_LIBRARIES})
target_link_libraries(SipClient ${CURL_LIBRARIES})

message(STATUS "===========OSIP2_INCLUDE_DIR:${OSIP2_LIBRARIES}")
message(STATUS "===========OSIP2_INCLUDE_DIR:${OSIP_PARSER2_LIBRARIES}")
message(STATUS "===========OSIP2_INCLUDE_DIR:${EXOSIP2_LIBRARIES}")

target_link_libraries(SipClient   ${OSIP2_LIBRARIES})
target_link_libraries(SipClient   ${OSIP_PARSER2_LIBRARIES})
target_link_libraries(SipClient   ${EXOSIP2_LIBRARIES})



