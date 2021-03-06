cmake_minimum_required(VERSION 3.12)
project(cs3103_p2p)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

set(BUILD_TRACKER tracker)
set(SOURCE_FILES_TRACKER src/tracker/start_tracker.cpp src/tracker/tracker.cpp src/tracker/tracker.h
        src/core/core_functions.cpp src/core/core_functions.h src/core/p2p_request_response_packet.h src/core/tracker_entries.cpp src/core/tracker_entries.h)
add_executable(${BUILD_TRACKER} ${SOURCE_FILES_TRACKER})

set(BUILD_CLIENT p2p_client)
set(SOURCE_FILES_CLIENT src/p2p_client/start_p2p_client.cpp src/p2p_client/p2p_client.cpp src/p2p_client/p2p_client.h
        src/core/core_functions.cpp src/core/core_functions.h src/core/p2p_request_response_packet.h src/core/storage.h src/core/storage.cpp
        src/p2p_client/p2p_client_helper.cpp src/p2p_client/p2p_client_helper.h
        src/core/tracker_entries.cpp src/core/tracker_entries.h
        src/p2p_server/p2p_server.cpp src/p2p_server/p2p_server.h)
add_executable(${BUILD_CLIENT} ${SOURCE_FILES_CLIENT})
