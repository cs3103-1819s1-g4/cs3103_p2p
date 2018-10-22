cmake_minimum_required(VERSION 3.12)
project(cs3103_p2p)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_executable(tracker src/tracker/start_server.cpp src/core/core_functions.cpp src/tracker/main_server.cpp src/tracker/main_server.h src/core/core_functions.cpp src/core/core_functions.h src/core/P2P_proto_packet.h src/core/tracker_entries.h)