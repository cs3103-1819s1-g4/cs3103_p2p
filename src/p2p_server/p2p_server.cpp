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
    time_listen_sock.tv_sec = 1000000;
    time_listen_sock.tv_usec = 0;
    FD_ZERO(&read_fd_listen_sock);
    FD_SET(listen_sock, &read_fd_listen_sock); // always look for connection attempts

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
            cout << "P2P client server listening...\n";

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

    size_t chunk_size;

    pair<string, string> filename_chunk_no_pair = parse_packet();
    string filename = filename_chunk_no_pair.first;
    int chunk_no = strtol(filename_chunk_no_pair.second.c_str(), nullptr, 10);

    if(storage->getChunk(chunk_buffer, filename, chunk_no, &chunk_size) != -1) {
        cout << "Obtained filename and chunk no from storage: " << filename << "\t" << chunk_no << "\n";

        strcpy_s(send_buffer, chunk_size, chunk_buffer);

        if(sendto(listen_sock, send_buffer, strlen(send_buffer), 0, (sockaddr *)&client_addr, sin_size) == SOCKET_ERROR) {
            cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
            WSACleanup();
            return false;
        }
        cout << "Successfully sent a chunk\tFile: " << filename << "\tChunk no: "
             << chunk_no << "\nPeer IP address: " << inet_ntoa(client_addr.sin_addr) << "\tPort no: "
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

pair<string, string> P2P_Server::parse_packet() {

    /* Variables to tokenise string*/
    string recv_buffer_str(recv_buffer);
    vector<string> str_token;
    stringstream check1(recv_buffer);
    string intermediate;

    while(getline(check1, intermediate, ' ')) {
        str_token.push_back(intermediate);
    }

    //assert((str_token[0] == "DOWNLOAD") == 0);

    return {str_token[1], str_token[2]};
};

// TODO: use listening socket or have a seperate socket
bool P2P_Server::get_public_ip_stun() {

    /* Variables for iterating */
    string stun_serv_ip;
    uint16_t stun_serv_port = NULL;

    /* Variables to communicate with STUN server */
    struct sockaddr_in stun_serv_addr{};
    bool chosen = false;
    Stun_Pkt_Handler stun_pkt_handler;
    unsigned char stun_pkt[STUN_PKT_LEN];
    int bytes_recv, status;

    ZeroMemory(&stun_serv_addr, sizeof(stun_serv_addr));
    stun_serv_addr.sin_family = AF_INET;

    for(auto it : STUN_SERV_VECTOR) {
       stun_serv_ip = it.first;
       stun_serv_port = it.second;
        //       struct hostent *he = gethostbyname(stun_serv_ip.c_str());
        //       struct in_addr addr;
        //       // for converstion to IP string
        //       addr.s_addr = *(u_long *) he->h_addr_list[0];
        //       status = inet_pton(AF_INET, inet_ntoa(addr), &stun_serv_addr.sin_addr);
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
}


bool P2P_Server::get_public_ip_stun2(char * return_ip_port, char * private_server_port) {
    cout<< endl << "Start of get_public_ip_stun2 function" << endl;

    string stun_serv_name;
    uint16_t stun_serv_port = NULL;
    struct in_addr addr;

    struct sockaddr_in stun_serv_addr{};
    bool chosen = false;
    int status;

    ZeroMemory(&stun_serv_addr, sizeof(stun_serv_addr));
    stun_serv_addr.sin_family = AF_INET;

    for(auto it : STUN_SERV_VECTOR) {
        stun_serv_name = it.first;
        stun_serv_port = it.second;
        struct hostent *he = gethostbyname(stun_serv_name.c_str());
        // for converstion to IP string
        addr.s_addr = *(u_long *) he->h_addr_list[0];
        status = inet_pton(AF_INET, inet_ntoa(addr), &stun_serv_addr.sin_addr);
        // Check if STUN ip address is valid
        if(status == 1) {
            chosen = true;
            break;
        }
    }


    int n = 0;
    string stun_serv_ip;
    stun_serv_ip = inet_ntoa(addr);
    n = stun_xor_addr(stun_serv_ip.c_str(),stun_serv_port,atoi(private_server_port),return_ip_port);
    if (n!=0)
        printf("STUN req error : %d\n",n);
    else
        printf("public ip:port = %s\n",return_ip_port);

    return false;
}

int P2P_Server::stun_xor_addr(const char * stun_server_ip,short stun_server_port,short local_port,char * return_ip_port)
{

    struct sockaddr_in servaddr;
    const int MAXLINE = 200;
    unsigned char buf[MAXLINE];
    int i;
    unsigned char bindingReq[20];

    short attr_type;
    short attr_length;
    short port;
    short n;


    // server
    memset(&servaddr, 0, sizeof(servaddr)); //sets all bytes of servaddr to 0
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, stun_server_ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(stun_server_port);

    printf("socket opened to  %s:%d  at local port %d\n",stun_server_ip,stun_server_port,local_port);

    //## first bind
    * (short *)(&bindingReq[0]) = htons(0x0001);    // stun_method
    * (short *)(&bindingReq[2]) = htons(0x0000);    // msg_length
    * (int *)(&bindingReq[4])   = htonl(0x2112A442);// magic cookie

    *(int *)(&bindingReq[8]) = htonl(0x63c7117e);   // transacation ID
    *(int *)(&bindingReq[12])= htonl(0x0714278f);
    *(int *)(&bindingReq[16])= htonl(0x5ded3221);


    // Send stun packet
    if(sendto(listen_sock, (char *)bindingReq, sizeof(bindingReq), 0, (struct sockaddr *)&servaddr, sizeof(servaddr))
       == SOCKET_ERROR) {
        cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    Sleep(1);

    if(recvfrom(listen_sock,(char *)buf,MAXLINE, 0, nullptr, nullptr) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << "\tReceive at socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }


    if (*(short *)(&buf[0]) == htons(0x0101))
    {
        printf("STUN binding resp: success !\n");

        // parse XOR
        n = htons(*(short *)(&buf[2]));
        i = 20;
        while(i<sizeof(buf))
        {
            attr_type = htons(*(short *)(&buf[i]));
            attr_length = htons(*(short *)(&buf[i+2]));
            if (attr_type == 0x0020)
            {
                // parse : port, IP

                port = ntohs(*(short *)(&buf[i+6]));
                port ^= 0x2112;

                sprintf(return_ip_port,"%d.%d.%d.%d:%d",buf[i+8]^0x21,buf[i+9]^0x12,buf[i+10]^0xA4,buf[i+11]^0x42,port);

                break;
            }
            i += (4  + attr_length);
        }
    }


    return 0;
}