
vector< pair<string, uint16_t> > STUN_SERV_VECTOR {make_pair("stun.l.google.com", 19305),
                                                   make_pair("stun1.l.google.com", 19305),
                                                   make_pair("stun2.l.google.com", 19305),
                                                   make_pair("stun3.l.google.com", 19305),
                                                   make_pair("stun4.l.google.com", 19305)};

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

    for (auto it : STUN_SERV_VECTOR) {
        stun_serv_ip = it.first;
        stun_serv_port = it.second;
        //       struct hostent *he = gethostbyname(stun_serv_ip.c_str());
        //       struct in_addr addr;
        //       // for converstion to IP string
        //       addr.s_addr = *(u_long *) he->h_addr_list[0];
        //       status = inet_pton(AF_INET, inet_ntoa(addr), &stun_serv_addr.sin_addr);
        status = inet_pton(AF_INET, stun_serv_ip.c_str(), &stun_serv_addr.sin_addr);
        // Check if STUN ip address is valid
        if (status == 1) {
            chosen = true;
            break;
        }
    }

    if (!chosen) {
        cout << "All stun servers were invalid, try other servers instead" << "\n";
        return false;
    }

    stun_serv_addr.sin_port = htons(stun_serv_port);

    // Create first bind packet
    stun_pkt_handler.create_first_bind_pkt(stun_pkt);

    print_server(&stun_serv_addr, to_string(stun_serv_port), "Attempting to communicate with stun server");
    cout.flush();

    // Send stun packet
    if (sendto(listen_sock, (char *) stun_pkt, sizeof(stun_pkt), 0, (struct sockaddr *) &stun_serv_addr,
               sizeof(stun_serv_addr))
        == SOCKET_ERROR) {
        cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    Sleep(1);

    if ((bytes_recv = recvfrom(listen_sock, recv_buffer, MAX_BUFFER_LEN, 0, nullptr, nullptr)) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << "\tReceive at socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    recv_buffer[bytes_recv] = '\0';

    if ((p2p_server_public_port = stun_pkt_handler.parse_request(recv_buffer, bytes_recv, &p2p_server_public_ip)) ==
        0) {
        cout << "Stun request binding failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    cout << "P2P client server's public ip:port is " << inet_ntoa(p2p_server_public_ip) << ":"
         << to_string(p2p_server_public_port) << "\n";

    return true;
}


bool P2P_Server::get_public_ip_stun2(char *return_ip_port, char *private_server_port) {
    cout << endl << "Start of get_public_ip_stun2 function" << endl;

    string stun_serv_name;
    uint16_t stun_serv_port = NULL;
    struct in_addr addr;

    struct sockaddr_in stun_serv_addr{};
    bool chosen = false;
    int status;

    ZeroMemory(&stun_serv_addr, sizeof(stun_serv_addr));
    stun_serv_addr.sin_family = AF_INET;

    for (auto it : STUN_SERV_VECTOR) {
        stun_serv_name = it.first;
        stun_serv_port = it.second;
        struct hostent *he = gethostbyname(stun_serv_name.c_str());
        // for converstion to IP string
        addr.s_addr = *(u_long *) he->h_addr_list[0];
        status = inet_pton(AF_INET, inet_ntoa(addr), &stun_serv_addr.sin_addr);
        // Check if STUN ip address is valid
        if (status == 1) {
            chosen = true;
            break;
        }
    }


    int n = 0;
    string stun_serv_ip;
    stun_serv_ip = inet_ntoa(addr);
    n = stun_xor_addr(stun_serv_ip.c_str(), stun_serv_port, atoi(private_server_port), return_ip_port);
    if (n != 0)
        printf("STUN req error : %d\n", n);
    else
        printf("public ip:port = %s\n", return_ip_port);

    return false;
}

int
P2P_Server::stun_xor_addr(const char *stun_server_ip, short stun_server_port, short local_port, char *return_ip_port) {

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

    printf("socket opened to  %s:%d  at local port %d\n", stun_server_ip, stun_server_port, local_port);

    //## first bind
    *(short *) (&bindingReq[0]) = htons(0x0001);    // stun_method
    *(short *) (&bindingReq[2]) = htons(0x0000);    // msg_length
    *(int *) (&bindingReq[4]) = htonl(0x2112A442);// magic cookie

    *(int *) (&bindingReq[8]) = htonl(0x63c7117e);   // transacation ID
    *(int *) (&bindingReq[12]) = htonl(0x0714278f);
    *(int *) (&bindingReq[16]) = htonl(0x5ded3221);


    // Send stun packet
    if (sendto(listen_sock, (char *) bindingReq, sizeof(bindingReq), 0, (struct sockaddr *) &servaddr, sizeof(servaddr))
        == SOCKET_ERROR) {
        cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }

    Sleep(1);

    if (recvfrom(listen_sock, (char *) buf, MAXLINE, 0, nullptr, nullptr) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << "\tReceive at socket failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return false;
    }


    if (*(short *) (&buf[0]) == htons(0x0101)) {
        printf("STUN binding resp: success !\n");

        // parse XOR
        n = htons(*(short *) (&buf[2]));
        i = 20;
        while (i < sizeof(buf)) {
            attr_type = htons(*(short *) (&buf[i]));
            attr_length = htons(*(short *) (&buf[i + 2]));
            if (attr_type == 0x0020) {
                // parse : port, IP

                port = ntohs(*(short *) (&buf[i + 6]));
                port ^= 0x2112;

                sprintf(return_ip_port, "%d.%d.%d.%d:%d", buf[i + 8] ^ 0x21, buf[i + 9] ^ 0x12, buf[i + 10] ^ 0xA4,
                        buf[i + 11] ^ 0x42, port);

                break;
            }
            i += (4 + attr_length);
        }
    }


    return 0;
}