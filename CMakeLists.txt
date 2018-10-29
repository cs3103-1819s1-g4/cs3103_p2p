cmake_minimum_required(VERSION 3.12)
project(cs3103_p2p)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

#set(BOOST_ROOT "C:/boost_1_67_0")
#find_package(BOOST COMPONENTS REQUIRED serialization)
#
#if(Boost_FOUND)
#    include_directories(${Boost_INCLUDE_DIRS})
#
#    target_link_libraries(p2p_client ${Boost_LIBRARIES})
#endif()
#
#if(NOT Boost_FOUND)
#    message(FATAL_ERROR "Cannot find Boost!")
#endif()

set(BUILD_TRACKER tracker)
set(SOURCE_FILES_TRACKER src/tracker/start_tracker.cpp src/tracker/tracker.cpp src/tracker/tracker.h
        src/core/core_functions.cpp src/core/core_functions.h src/core/p2p_request_response_packet.h src/core/tracker_entries.cpp src/core/tracker_entries.h)
add_executable(${BUILD_TRACKER} ${SOURCE_FILES_TRACKER})

set(BUILD_CLIENT p2p_client)
set(SOURCE_FILES_CLIENT src/p2p_client/start_p2p_client.cpp src/p2p_client/p2p_client.cpp src/p2p_client/p2p_client.h
        src/core/core_functions.cpp src/core/core_functions.h src/core/p2p_request_response_packet.h src/core/storage.h src/core/storage.cpp)
add_executable(${BUILD_CLIENT} ${SOURCE_FILES_CLIENT})

set(BUILD_P2P_SERVER p2p_server)
set(SOURCE_FILES_P2P_SERVER src/p2p_server/start_p2p_server.cpp src/p2p_server/p2p_server.cpp src/p2p_server/p2p_server.h src/core/p2p_request_response_packet.h
        src/core/core_functions.cpp src/core/core_functions.h src/core/storage.h src/core/storage.cpp src/p2p_server/start_p2p_server.cpp src/core/p2p_request.cpp src/core/p2p_request.h
        src/p2p_server/stun_pkt_handler.cpp src/p2p_server/stun_pkt_handler.h src/p2p_server/stun_pkt_handler.tpp)
add_executable(${BUILD_P2P_SERVER} ${SOURCE_FILES_P2P_SERVER})
