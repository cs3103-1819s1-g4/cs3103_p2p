#include "p2p_server.h"

void P2P_Server::stop() {

    if (listen_sock != INVALID_SOCKET) {
        closesocket(listen_sock);
        listen_sock = INVALID_SOCKET;
    }
}

bool P2P_Server::start(const char *port) {

    stop();

    struct addrinfo *result = nullptr, hints{};
    int status;

    ZeroMemory(&result, sizeof(result));
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    status = getaddrinfo(inet_ntoa(p2p_server_private_ip), port, &hints, &result);
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

    if (::bind(serv_sock, result->ai_addr, (int) result->ai_addrlen) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket.\n";
        freeaddrinfo(result);
        closesocket(serv_sock);
        return false;
    }

    print_server((struct sockaddr_in *) result->ai_addr, port, "P2P client server");
    std::cout.flush();

    // We don't need this info any more
    freeaddrinfo(result);
    listen_sock = serv_sock;

    return true;
}

bool P2P_Server::listen() {

    // variables to store client's ip address
    struct sockaddr_in client_addr{};
    int status, sin_size = sizeof(client_addr);

    // variables for socket listening descriptors
    struct timeval time_listen_sock{};
    fd_set read_fd_listen_sock;
    int bytes_recv;

    //configure file descriptors and timeout
    time_listen_sock.tv_sec = 50;
    time_listen_sock.tv_usec = 0;
    FD_ZERO(&read_fd_listen_sock);
    FD_SET(listen_sock, &read_fd_listen_sock); // always look for connection attempts

    cout << "P2P client server listening...\n";
    if( (status = select(listen_sock + 1, &read_fd_listen_sock, nullptr, nullptr, &time_listen_sock)) == SOCKET_ERROR ) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Select with socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    } else if (status == 0) {
        std::cout << "Time Limit expired at select\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    } else {
        while(true) {
            if ((bytes_recv = recvfrom(listen_sock, recv_buffer, MAX_BUFFER_LEN, 0, (struct sockaddr *)&client_addr,
                                       &sin_size)) == SOCKET_ERROR) {
                cout << "[ERROR]: " << WSAGetLastError() << "\tReceive at socket failed\n";
                WSACleanup();
                return false;
            }

            cout << "Received a datagram packet from:" << inet_ntoa(client_addr.sin_addr) << "\tport no: "
                      << ntohs(client_addr.sin_port) << "\n";

            recv_buffer[bytes_recv] = '\0';

            process_request(client_addr, sin_size);
        }
    }
}

bool P2P_Server::process_request(sockaddr_in client_addr, int sin_size) {

    auto *request = (struct P2P_request_pkt *)recv_buffer;
    size_t chunk_size;

    assert(request->flag == 3);

    if(storage.getChunk(chunk_buffer, request->file_name, request->chunk_no, &chunk_size) != -1) {

        strcpy_s(send_buffer, chunk_size - FIXED_CHUNK_HEADER_SIZE, chunk_buffer + FIXED_CHUNK_HEADER_SIZE);

        if(sendto(listen_sock, send_buffer, strlen(send_buffer), 0, (sockaddr *)&client_addr, sin_size) == SOCKET_ERROR) {
            cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
            WSACleanup();
            return false;
        }
        cout << "Successfully sent a chunk\nFile: " << request->file_name << "\tChunk no: "
             << request->chunk_no << "\nPeer IP address: " << inet_ntoa(client_addr.sin_addr) << "\tPort no: "
             << ntohs(client_addr.sin_port) << "\n";

    } else {
        strcpy_s(send_buffer, sizeof(CHUNK_NOT_FOUND_ERROR), CHUNK_NOT_FOUND_ERROR);

        if (sendto(listen_sock, send_buffer, strlen(send_buffer), 0, (sockaddr *) &client_addr, sin_size) ==
            SOCKET_ERROR) {
            cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
            WSACleanup();
            return false;
        }
    }
    return true;
}

/* 1 socket for public ip, 1 socket for private ip? */
bool P2P_Server::get_public_ip_stun() {

    WSADATA wsock;
    int status = WSAStartup(MAKEWORD(2,2),&wsock);

    if ( status != 0)
        cout << "[ERROR]: " << status << " Unable to start Winsock.\n";

    /* Variables for iterating */
    string stun_serv_ip;
    uint16_t stun_serv_port = NULL;

    /* Variables to communicate with STUN server */
    struct sockaddr_in stun_serv_addr{};
    bool chosen = false;
    STUN_Pkt_Handler stun_pkt_handler;
    unsigned char stun_pkt[STUN_PKT_LEN];
    int bytes_recv;

    ZeroMemory(&stun_serv_addr, sizeof(stun_serv_addr));
    stun_serv_addr.sin_family = AF_INET;

    for(auto it : STUN_SERV_VECTOR) {
       stun_serv_ip = it.first;
       stun_serv_port = it.second;
       status = inet_pton(AF_INET, stun_serv_ip.c_str(), &stun_serv_addr.sin_addr);
        // Check if STUN ip address is valid
        if(status == 1) {
           chosen = true;
           break;
       }
    }

    if(!chosen) {
        cout << "All stun servers were invalid, try other servers instead" << "\n";
        return false;
    }

    stun_serv_addr.sin_port = htons(stun_serv_port);

    // Create first bind packet
    stun_pkt_handler.create_first_bind_pkt(stun_pkt);

    print_server(&stun_serv_addr, to_string(stun_serv_port), "Attempting to communicate with stun server");
    cout.flush();

    // Send stun packet
    if(sendto(listen_sock, (char *)stun_pkt, sizeof(stun_pkt), 0, (struct sockaddr *)&stun_serv_addr, sizeof(stun_serv_addr))
            == SOCKET_ERROR) {
        cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    Sleep(1);

    if((bytes_recv = recvfrom(listen_sock, recv_buffer, MAX_BUFFER_LEN, 0, nullptr, nullptr)) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << "\tReceive at socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    recv_buffer[bytes_recv] = '\0';

    if((p2p_server_public_port = stun_pkt_handler.parse_request(recv_buffer, bytes_recv, &p2p_server_public_ip)) == 0) {
        cout << "Stun request binding failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    cout << "P2P client server's public ip:port is " << inet_ntoa(p2p_server_public_ip) << ":"
    << to_string(p2p_server_public_port) << "\n";

    return true;
};