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

bool P2P_Server::listen(string& signal_public_ip) {

    thread keep_alive_thread(&P2P_Server::keep_alive_udp, this);
    keep_alive_thread.detach();

    cout << "P2P server listening..." << "\n";
    cout.flush();

    while (true) {

        char *recv_buffer;
        recv_buffer = (char *)malloc(MAX_BUFFER_LEN);

        int bytes_recieved = read_from_signal_public_ip(recv_buffer,MAX_BUFFER_LEN);

        string isGetIpStr = "yourIP";
        string recvStrToCheck = string(recv_buffer,32);

        if(recvStrToCheck.substr(0,6) == isGetIpStr) {
            // is get public ip request
            recv_buffer[bytes_recieved] = '\0';
            string ipPort = string(recv_buffer+6);
            //cout<<"myipis " << ipPort <<endl;
            signal_public_ip = ipPort;
        } else if(bytes_recieved != -1) {
            cout << "P2P server received request..." << "bytes recieved-"<< bytes_recieved << "\n";
            cout.flush();

            recv_buffer[bytes_recieved] = '\0';
            string request = string(recv_buffer);

            thread client_thread(&P2P_Server::process_request, this, request);

            client_thread.detach();
        } else {
            cout << "P2P server error UDP..." << endl << recv_buffer << "\n";
            cout.flush();

        }
    }

}

bool P2P_Server::keep_alive_udp(){
    while(true){
        send_signaller_public_ip_port();
        Sleep(5000);
    }
}


bool P2P_Server::process_request(string request) {

    cout <<  "req-----"<< request << endl;

    // Each TCP connection has an individual send and receive buffer
    size_t chunk_size;
    char *send_buffer; //, *recv_buffer;
    int iresult, chunk_no;

    send_buffer = (char *)malloc(MAX_BUFFER_LEN);

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
        cout << "sent chunk" << "\n";
    } else {
        // If chunk cannot be found, send CHUNK_NOT_FOUND_ERROR to client
        strcpy_s(send_buffer, sizeof(CHUNK_NOT_FOUND_ERROR), CHUNK_NOT_FOUND_ERROR);

        if (send_to_TURN_public_ip(client_public_ip_port, send_buffer, sizeof(CHUNK_NOT_FOUND_ERROR)) == -1) {
            cout << "[ERROR]: " << WSAGetLastError() << "\tSend to socket failed\n";
            return false;
        }
    }


    cout << "Connection with client has finished successfully" << "\n";
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


    return {str_token[1], str_token[2], str_token[3]};
};

bool P2P_Server::setupSocketForSignallerServer(){
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

    signal_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (signal_sock == INVALID_SOCKET) {
        cout << "[ERROR]: " << WSAGetLastError() << " Unable to create Socket.\n";
        freeaddrinfo(result);
        return false;
    }

    if (::bind(signal_sock, result->ai_addr, (int) result->ai_addrlen) == SOCKET_ERROR) {
        cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket.\n";
        freeaddrinfo(result);
        closesocket(signal_sock);
        return false;
    }

    freeaddrinfo(result);

    return true;
}


void P2P_Server::send_signaller_public_ip_port() {
    std::string signal_server_ip = "18.136.118.72";
    int signal_server_port = 6883;
    struct sockaddr_in servaddr;
    const int MAXLINE = 32;
    //char buf[MAXLINE];
    char bindingReq[20];
    strcpy(bindingReq, "getPublic");

    // server
    memset(&servaddr, 0, sizeof(servaddr)); //sets all bytes of servaddr to 0
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, signal_server_ip.c_str(), &servaddr.sin_addr);
    servaddr.sin_port = htons(signal_server_port);

    sendto(signal_sock, (char *)bindingReq, sizeof(bindingReq), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

}



int P2P_Server::read_from_signal_public_ip(char* data, int max_bytes_of_data_buffer_allocated){
    int bytes_recieved = recvfrom(signal_sock,data,max_bytes_of_data_buffer_allocated,0, nullptr, nullptr);
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
