#ifndef CS3103_P2P_MAIN_SERVER_H
#define CS3103_P2P_MAIN_SERVER_H

#include <iostream>
#include <cstdio>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <ipmib.h>
#include <iphlpapi.h>

int init_server(SOCKET &server_sock, IN_ADDR &server_IP);

class Main_Server {

private:
    bool online;
    SOCKET sock;
    IN_ADDR *server_IP;
    void get_local_IP(IN_ADDR &IP);
public:
    Main_Server();
    ~Main_Server();
    bool start(const char *port);
    void stop();
};

#endif //CS3103_P2P_MAIN_SERVER_H
