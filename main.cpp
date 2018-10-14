#include <iostream>
#include <cstdio>
#include "main_server.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_SERVER_PORT "7777"

using namespace std;

int main() {

    auto *main_server = new Main_Server();

    if (!main_server->start(DEFAULT_SERVER_PORT))
       return 1;
    else {

    }

    main_server->~Main_Server();
    return 0;
}
