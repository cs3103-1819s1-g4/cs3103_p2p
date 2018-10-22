#ifndef CS3103_P2P_MAIN_SERVER_H
#define CS3103_P2P_MAIN_SERVER_H

#include <process.h>
#include <WS2tcpip.h>
#include <winsock.h>

#include "../core/core_functions.h"

/**
 * global receive buffer within the scope of the main server for receiving datagram and passing data
 * to processing threads
 */
extern RecvBuffer recv_buffer;
extern bool recv_socket_active;
extern const unsigned int PACKET_SIZE;

class MainServer {

private:
    bool online;
    SOCKET listen_sock;
    IN_ADDR server_IP{};
public:
    /**
     * Constructor for Main Server
     */
    MainServer() : listen_sock(INVALID_SOCKET), online(false)   {

        WSADATA wsock;
        int status = WSAStartup(MAKEWORD(2,2),&wsock);

        if ( status != 0)
            std::cout << "[ERROR]: " << status << " Unable to start Winsock.\n";
        else
            online = true;

        get_private_IP(server_IP);
        recv_buffer.init_RecvBuffer(PACKET_SIZE);
    };
    ~MainServer() {
        stop();
        if (online)
            WSACleanup();
    };
    bool start(const char *port);
    void stop();
};

unsigned int __stdcall socket_recv_thread(void *data);
#endif //CS3103_P2P_MAIN_SERVER_H
