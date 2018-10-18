#include "main_server.h"

MainServer::MainServer() : listen_sock(INVALID_SOCKET), online(false) {

    WSADATA wsock;
    int status = WSAStartup(MAKEWORD(2,2),&wsock);

    if ( status != 0)
        std::cout << "[ERROR]: " << status << " Unable to start Winsock.\n";
    else
        online = true;

    ZeroMemory(&server_IP, sizeof(server_IP));
    get_private_IP(server_IP);
}

MainServer::~MainServer() {

    stop();
    if (online)
        WSACleanup();
}

void MainServer::stop() {

    if (listen_sock != INVALID_SOCKET) {
        closesocket(listen_sock);
        listen_sock = INVALID_SOCKET;
    }
}

bool MainServer::start(const char *port) {

    stop();

    struct addrinfo *result = nullptr, hints;
    int status;

    ZeroMemory(&result, sizeof (result));
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_UDP;

    status = getaddrinfo(inet_ntoa(server_IP), port, &hints, &result);
    if (status != 0) {
        std::cout << "[ERROR]: " << status << " Unable to get address info for Port " << port << ".\n";
        return false;
    }

    SOCKET serv_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_sock == INVALID_SOCKET) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to create Socket.\n";
        freeaddrinfo(result);
        return false;
    }

    int on = 1;
    if( (setsockopt(serv_sock, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on))) < 0 ) {
        perror("setsockopt");
        return false;
    }

    if (bind(serv_sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket.\n";
        freeaddrinfo(result);
        closesocket(serv_sock);
        return false;
    }

    print_main_server((struct sockaddr_in *)result->ai_addr, port);
    std::cout.flush();

    // We don't need this info any more
    freeaddrinfo(result);
    listen_sock = serv_sock;

    // start to receive from socket here using a thread
    auto socket_recv_handle = (HANDLE)_beginthreadex(nullptr, 0, &socket_recv_thread, &listen_sock, 0, nullptr);
    WaitForSingleObject(socket_recv_handle, INFINITE); // wait
    CloseHandle(socket_recv_handle);

    return true;
}

unsigned int __stdcall socket_recv_thread(void *data) {

    auto recv_buffer = new RecvBuffer(PACKET_SIZE);
    auto listen_sock = *(SOCKET *)data;
    struct sockaddr_in client_addr{};
    int status, sin_size = sizeof(client_addr);

    // variables for socket listening descriptors
    struct timeval time_listen_sock{};
    fd_set read_fd_listen_sock;

    ZeroMemory(&client_addr, sizeof(client_addr));
    recv_socket_active = true;

    //configure file descriptors and timeout
    time_listen_sock.tv_sec = 3;
    time_listen_sock.tv_usec = 0;
    FD_ZERO(&read_fd_listen_sock);
    FD_SET(listen_sock, &read_fd_listen_sock); // always look for connection attempts

    if( (status = select(listen_sock + 1, &read_fd_listen_sock, nullptr, nullptr, &time_listen_sock)) == SOCKET_ERROR ) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Select with socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    } else if (status == 0) {
        std::cout << "Time Limit expired at select\n";
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    } else {
        while(recv_socket_active) {
            recv_socket_active = recv_buffer->producer(listen_sock, client_addr, sin_size);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // sleep
        }
    }
    recv_buffer->~RecvBuffer();
    return 0;
}
