#include <iostream>
#include <cstdio>
#include "main_server.h"
#include "core/core_functions.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_SERVER_PORT "80"

using namespace std;

bool recv_socket_active;
const unsigned int Q_LEN = 100;
const unsigned int PACKET_SIZE = 2048;

int main() {

    auto main_server = new MainServer();

    if (!main_server->start(DEFAULT_SERVER_PORT))
       return 1;
    else {

    }
    main_server->~MainServer();
    std::cout << "Goodbye!";
    return 0;
}
