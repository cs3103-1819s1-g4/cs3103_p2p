#include "p2p_server.h"

#pragma comment(lib, "Bcrypt")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#define DEFAULT_P2P_SERVER_PORT "6881"

using namespace std;

int main() {

    auto p2p_server = new P2P_Server();

    cout << "Starting P2P server...\n";

    if (!p2p_server->start(DEFAULT_P2P_SERVER_PORT)) {
        cout << "Failed to start server";
        return 1;
    } else {
        p2p_server->get_public_ip_stun();
        char ip_port[50];
        p2p_server->get_public_ip_stun2(ip_port, DEFAULT_P2P_SERVER_PORT);
        p2p_server->listen();
    }

    p2p_server->~P2P_Server();
    std::cout << "Goodbye!";
    return 0;
}
