#ifndef CS3103_P2P_MAIN_SERVER_H
#define CS3103_P2P_MAIN_SERVER_H

#include <process.h>
#include <WS2tcpip.h>
#include <winsock.h>

#include "core/core_functions.h"

extern bool recv_socket_active;

class MainServer {

private:
    bool online;
    SOCKET listen_sock;
    IN_ADDR server_IP;
public:
    MainServer();
    ~MainServer();
    bool start(const char *port);
    void stop();
};

unsigned int __stdcall socket_recv_thread(void *data);

#endif //CS3103_P2P_MAIN_SERVER_H
