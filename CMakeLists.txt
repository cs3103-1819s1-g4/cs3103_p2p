cmake_minimum_required(VERSION 3.12)
project(cs3103_p2p)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
add_executable(cs3103_p2p main.cpp core/core_functions.cpp main_server.cpp main_server.h core/core_functions.cpp core/core_functions.h)
