#include "p2p_server.h"

void P2P_Server::stop() {

    if (listen_sock != INVALID_SOCKET) {
        closesocket(listen_sock);
        listen_sock = INVALID_SOCKET;
    }
}

bool P2P_Server::start(const char *port) {
    //testTURN();
    stop();

    struct addrinfo *result = nullptr, hints{};
    int status;

    ZeroMemory(&result, sizeof(result));
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

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
    // struct sockaddr_in client_addr{};
    // int sin_size;
    // SOCKET client_sock;

    // if (::listen(listen_sock, MAX_CONNECTIONS) == SOCKET_ERROR) {
    //     cout << "[ERROR]: " << WSAGetLastError() << " Listen sock failed\n";
    //     closesocket(listen_sock);
    //     WSACleanup();
    //     return false;
    // }

    cout<< "THIS IS THE PUBLIC SIGNAL PORT:" << get_signaller_public_ip_port() << endl;

    while (true) {
        //sin_size = sizeof(client_addr);

        cout << "P2P server listening..." << "\n";
        cout.flush();
        // client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &sin_size);
        // // Don't exit, continue to listen
        // if(client_sock == INVALID_SOCKET) {
        //     cout << "[ERROR]: " << WSAGetLastError() << " Accept client failed\n"
        //          << "Might have exceeded max connections allowed\n";
        // }


        char *recv_buffer;
        recv_buffer = (char *)malloc(MAX_BUFFER_LEN);

        int bytes_recieved = read_from_signal_public_ip(recv_buffer,MAX_BUFFER_LEN);

        string request = string(recv_buffer);

        // cout << "Received a connection from:" << inet_ntoa(client_addr.sin_addr) << "\tport no: "
        //      << ntohs(client_addr.sin_port) << "\n";


        thread client_thread(&P2P_Server::process_request, this, request);

        //client_thread.detach();
    }

    WSACleanup();
    closesocket(listen_sock);
}

bool P2P_Server::process_request(string request) {

    // Each TCP connection has an individual send and receive buffer
    size_t chunk_size;
    char *send_buffer; //, *recv_buffer;
    int iresult, chunk_no;

    //recv_buffer = (char *)malloc(MAX_BUFFER_LEN);
    send_buffer = (char *)malloc(MAX_BUFFER_LEN);

    // if(recv_buffer == nullptr || send_buffer == nullptr) {
    //     cout << "[ERROR]: " << GetLastError() << " Not enough memory to allocate to send/recv buffer\n";
    //     closesocket(client_sock);
    //     return false;
    // }

    char req[128];
    strcpy(req,request.c_str());
    // Parse packet to get filename and chunk_no requested
    tuple<string, string, string> filename_chunk_no_pair = parse_packet(req);
    string filename = get<0>(filename_chunk_no_pair);
    chunk_no = strtol(get<1>(filename_chunk_no_pair).c_str(), nullptr, 10);
    string client_public_ip_port = get<2>(filename_chunk_no_pair);

    if (storage->getChunk(send_buffer, filename, chunk_no, &chunk_size) != -1) {
        cout << "Obtained filename and chunk no from storage: " << filename << "\t" << chunk_no << "\n";

        // If get chunk from storage is successful, send data
        iresult = send_to_TURN_public_ip(client_public_ip_port, send_buffer, chunk_size);
        if (iresult == -1) {
            cout << "[ERROR]: " << WSAGetLastError() << "\tSend to TURN client socket failed\n";
            return false;
        }
        cout << "Bytes sent: " << iresult << "\n";
    } else {
        // If chunk cannot be found, send CHUNK_NOT_FOUND_ERROR to client
        strcpy_s(send_buffer, sizeof(CHUNK_NOT_FOUND_ERROR), CHUNK_NOT_FOUND_ERROR);

        if (send_to_TURN_public_ip(client_public_ip_port, send_buffer, sizeof(CHUNK_NOT_FOUND_ERROR)) == -1) {
            cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
            return false;
        }
    }


    // iresult = shutdown(client_sock, SD_SEND);
    // if(iresult == SOCKET_ERROR) {
    //     cout << "[ERROR]: " << WSAGetLastError() << "\tShutdown with client failed\n";
    //     closesocket(client_sock);
    //     return false;
    // }

    cout << "Connection with client has finished successfully" << "\n";
    //closesocket(client_sock);
    return true;
}

tuple<string, string, string> P2P_Server::parse_packet(char *recv_buffer) {

    /* Variables to tokenise string*/
    string recv_buffer_str(recv_buffer);
    vector<string> str_token;
    stringstream check1(recv_buffer);
    string intermediate;

    while (getline(check1, intermediate, ' ')) {
        str_token.push_back(intermediate);
    }

    //assert((str_token[0] == "DOWNLOAD") == 0);

    return {str_token[1], str_token[2], str_token[3]};
};

bool P2P_Server::setupSocketForSignallerServer(SOCKET* serv_sock){
    string private_ip = inet_ntoa(p2p_server_private_ip);
    struct addrinfo *result = nullptr, hints{};
    int status;

    ZeroMemory(&result, sizeof(result));
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    string port = "6889";
    status = getaddrinfo(private_ip.c_str(), port.c_str(), &hints, &result);
    if (status != 0) {
        cout << "[ERROR]: " << status << " Unable to get address info for Port " << port << ".\n";
        return false;
    }

    *serv_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (*serv_sock == INVALID_SOCKET) {
        cout << "[ERROR]: " << WSAGetLastError() << " Unable to create Socket.\n";
        freeaddrinfo(result);
        return false;
    }

    if (::bind(*serv_sock, result->ai_addr, (int) result->ai_addrlen) == SOCKET_ERROR) {
        cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket.\n";
        freeaddrinfo(result);
        closesocket(*serv_sock);
        return false;
    }

    freeaddrinfo(result);
    signaller_sock = *serv_sock;

    return true;
}


string P2P_Server::get_signaller_public_ip_port() {
    std::string signal_server_ip = "18.136.118.72";
    int signal_server_port = 6883;
    struct sockaddr_in servaddr;
    const int MAXLINE = 32;
    char buf[MAXLINE];
    char bindingReq[20];
    strcpy(bindingReq, "getPublic");

    // server
    memset(&servaddr, 0, sizeof(servaddr)); //sets all bytes of servaddr to 0
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, signal_server_ip.c_str(), &servaddr.sin_addr);
    servaddr.sin_port = htons(signal_server_port);

    sendto(signaller_sock, (char *)bindingReq, sizeof(bindingReq), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Sleep(1000);
    // TODO timeout
    int recv = recvfrom(signaller_sock,(char *)buf,MAXLINE, 0, nullptr, nullptr);
    buf[recv] = '\0';
    return string(buf);
}

int P2P_Server::read_from_signal_public_ip(char* data, int max_bytes_of_data_buffer_allocated){
    int bytes_recieved = recv(signaller_sock,data,max_bytes_of_data_buffer_allocated,0);
    return bytes_recieved;
}

int P2P_Server::send_to_TURN_public_ip(string public_TURN_ip_of_dest, char* data, int num_bytes_of_data_to_send){
    string signal_server_ip = "18.136.118.72";
    int bytes_recieved;
    string send_data = "getPublic";
    char recv_data[1024];
    struct hostent *host;
    struct sockaddr_in server_addr;

    host = gethostbyname(signal_server_ip.c_str());

    SOCKET sock = socket(AF_INET, SOCK_STREAM,0);
        if (sock == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(EXIT_FAILURE);
        }



    memset(&server_addr, 0, sizeof(server_addr)); //sets all bytes of servaddr to 0
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, signal_server_ip.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(6882);

    //connect to server at port 5000
    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1)
    {
        return -1;
    }


    string dataToSend = public_TURN_ip_of_dest;
    send(sock,dataToSend.c_str(),dataToSend.length(), 0);
    bytes_recieved = recv(sock,recv_data,1024,0);
    send(sock,data,num_bytes_of_data_to_send, 0);

    closesocket(sock);

    return 1;
}

//void P2P_Server::testTURN(){
////    setupSocketForSignallerServer();
////    string temp = get_signaller_public_ip_port();
////    cout<<temp<<endl;
////    char buff[8000];
////    int readed = read_from_signal_public_ip(buff,8000);
////    buff[readed] = '\0';
////    cout << buff << endl;
//
//     char buff[30];
//     char tempstr[] = "hello2shoe\0";
//     strcpy(buff,tempstr);
//    send_to_TURN_public_ip("175.156.181.183:5021",buff,10);
//
//;}