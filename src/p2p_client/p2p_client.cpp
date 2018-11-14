//
// Created by Jarrett on 22/10/2018.
//

#include "p2p_client.h"
#include <iostream>
#include <fstream>
#include "../core/storage.h"
#include "../core/p2p_request.h"
#include "../core/tracker_entries.h"
#include "p2p_client_helper.h"
#include <map>

#define MAX_BUFFER_SIZE 65536
#undef max

using namespace std;

// Winsock variables
SOCKET connect_socket;
SOCKET signaller_sock;
WSAData wsa_data;
struct addrinfo *result = nullptr,
        *ptr = nullptr,
        hints;
int iresult;

char recvbuf[MAX_BUFFER_SIZE];
map <int, tracker_peer_list_entry> peer_list;
string filename;

void p2p_client::display_menu() {
    printf("******P2P CLIENT******\n"
           "Enter options (1 to 5):\n"
           "\t1. Download a file\n"
           "\t2. Upload a file\n"
           "\t3. Query for a list of files available in tracker\n"
           "\t4. Query for a file available in tracker\n"
           "\t5. Quit\n"
           "Enter option: ");

}

int p2p_client::connection(const char *ip_addr, const char *port_num, bool is_tracker) {

    iresult = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (iresult != 0) {
        printf("WSAStartup failed with error: %d\n", iresult);
        exit(EXIT_FAILURE);
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    if (is_tracker) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    } else {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }


    iresult = getaddrinfo(ip_addr, port_num, &hints, &result);
    if ( iresult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iresult);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    for(ptr=result; ptr != nullptr ; ptr=ptr->ai_next) {

        connect_socket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (connect_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        break;
    }

    freeaddrinfo(result);

    if (connect_socket == INVALID_SOCKET && is_tracker) {
        printf("Unable to connect to tracker!\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // ASK UPDATED PEER LIST
    if (connect_socket == INVALID_SOCKET && !is_tracker) {
        printf("Unable to connect to p2p server! Trying again...\n");
        WSACleanup();
        return -1;
    }

    return 0;
}

void p2p_client::download_file(char *tracker_port, string filename) {

    // variables for socket listening descriptors
    struct timeval time_connect_socket{};
    fd_set read_fd_connect_socket;
    int sent_pkt_count = 0;
    bool recv_tracker_reply = false;

    this->connection(this->tracker_ip, tracker_port, true);

    string str = "REQUEST 1 " + filename;
    const char *buf = str.c_str();

    //configure file descriptors and timeout
    time_connect_socket.tv_sec = 2;
    time_connect_socket.tv_usec = 0;
    FD_ZERO(&read_fd_connect_socket);
    FD_SET(connect_socket, &read_fd_connect_socket); // always look for connection attempts

    while (sent_pkt_count != 3) {
        iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);
        sent_pkt_count++;
        if ((iresult = select(connect_socket + 1, &read_fd_connect_socket, nullptr, nullptr, &time_connect_socket))
            == SOCKET_ERROR) {
            cout << "[ERROR]: " << WSAGetLastError() << " Select with socket failed\n";
            closesocket(connect_socket);
            return;

            // If no reply from tracker, resend packet
        } else if (iresult == 0) {
            cout << "No reply from tracker, resending request, count = " << sent_pkt_count << "\n";
            continue;

        } else {
            iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);
            if (iresult == SOCKET_ERROR) {
                std::cout << "[ERROR]: " << WSAGetLastError() << "\tReceive at socket failed\n";
                closesocket(connect_socket);
                WSACleanup();
                return;
            }
            recvbuf[iresult] = '\0';
            recv_tracker_reply = true;
            break;
        }
    }

    closesocket(connect_socket);
    WSACleanup();

    // If failed to receive tracker reply, exit
    if (!recv_tracker_reply) {
        cout << "Failed to receive tracker reply, pls try again...\n";
        WSACleanup();
        return;
    }

    string recv_str(recvbuf);
    int peer_list_size = parse_peer_list(peer_list, recvbuf);

    memset(recvbuf, '\0', MAX_BUFFER_SIZE); // clears recvbuf

    // The following part deals with connection to p2p server
    // TODO: Connect to p2p_server.. almost done..
    string p2p_server_ip;
    string p2p_server_chunk_num;
    string p2p_server_port_num;
    vector<bool> check_downloaded_chunks(static_cast<unsigned int>(peer_list_size + 1));
//    for (auto i = 0; i < peer_list_size + 1; i++) {
//        check_downloaded_chunks[i] = false;
//    }
    check_downloaded_chunks[0] = NULL; // since chunk starts from #1; we will not use index 0.
    //int retry_count = 15;
    bool complete = false;
    while (!peer_list.empty()) {

        choose_random_server(peer_list, p2p_server_ip,
                p2p_server_chunk_num, p2p_server_port_num);

        cout << "Trying to obtain chunk number " + p2p_server_chunk_num << endl;

        // The chunk_num has already been downloaded; choose another chunk
        if (check_downloaded_chunks[stoi(p2p_server_chunk_num)]) {
            continue;
        } else {
            // Testing by printing
            cout << "Connecting to: " + p2p_server_ip + ", " + p2p_server_port_num << endl;

            // If the connection fails, try again...
            // if (this->connection(p2p_server_ip.c_str(), p2p_server_port_num.c_str(),
            //         false) == -1) {
            //     this->ask_updated_peer_list(DEFAULT_TRACKER_PORT, filename);
            //     continue;
            // }
            SOCKET recv_sock;
            string TURN_public_ip = connect_to_TURN_get_public_ip(&recv_sock);
            if(TURN_public_ip == ""){
                cout << "Error connecting to TURN sever, please try again" << endl;
                break;
            }

            str = "DOWNLOAD " + filename + " " + p2p_server_chunk_num + " " + TURN_public_ip;
            const char *buf_tcp = str.c_str();

            // Connect to server.
            // iresult = connect( connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
            // if (iresult == SOCKET_ERROR) {
            //     closesocket(connect_socket);
            //     connect_socket = INVALID_SOCKET;
            //     continue;
            // }
            // iresult = send(connect_socket, buf_tcp, strlen(buf_tcp), 0);

            send_to_signal_public_ip(p2p_server_ip, buf_tcp, strlen(buf_tcp));

            int recvSize;
            // recvSize = recv(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0);

            // TODO timeout this function
            recSize = read_from_TURN_public_ip(recv_sock,recv_buffer, MAX_BUFFER_SIZE);

            cout << "Received the chunk!" << endl;
            string temp(recvbuf);
            cout << temp << endl;

            // p2p_server will send me just the chunk data...
//            Storage storage("..\\download");
            (this->p2p_client_storage)->saveChunk(recvbuf, sizeof(char), recvSize, filename);

            closesocket(connect_socket);
            memset(recvbuf, '\0', MAX_BUFFER_SIZE); // clears recvbuf
//            peer_list.clear(); // clears peer_list
            WSACleanup();

            check_downloaded_chunks[stoi(p2p_server_chunk_num)] = true;

            // Once the chunk is downloaded, inform the tracker
            this->inform_tracker_downloaded_chunk(tracker_port, filename, p2p_server_chunk_num);
        }

        if((this->p2p_client_storage)->getFinalChunkNumber(filename) != -1)
        {
            complete = true;
            break;
        }
    }
    if(complete){
        printf("File Complete\n");
    } else
    {
        printf("File Incomplete\n");
    }

}

void p2p_client::query_list_of_files(char *tracker_port) {

    this->connection(this->tracker_ip, tracker_port, true);

    string str = "REQUEST 6";
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);

    string recv_str(recvbuf);

    // When tracker only returns "RESPONSE", means there are no files at tracker
    if (recv_str == "RESPONSE ") {
        cout << "No files found" << endl;
    } else {
        string space_delimiter = " ";
        recv_str.erase(0, recv_str.find(space_delimiter) + space_delimiter.length());
        cout << recv_str;
        // TODO: Maybe I have to parse the output to make it look nicer.
    }

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE); // clears recvbuf
    WSACleanup();
}

void p2p_client::query_file(char *tracker_port, string filename) {

//    assert(filename.length() < 256);

    this->connection(this->tracker_ip, tracker_port, true);

    string str = "REQUEST 7 " + filename;
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);
    string recv_str(recvbuf);
    string space_delimiter = " ";
    recv_str.erase(0, recv_str.find(space_delimiter) + space_delimiter.length());
    cout << recv_str << endl;

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::upload_file(char *tracker_port, string filename) {

    this->connection(this->tracker_ip, tracker_port, true);

    int chunk_no_buffer[MAX_BUFFER_SIZE];
    int num_of_chunks = p2p_client_storage->getArrOfChunkNumbers(chunk_no_buffer, MAX_BUFFER_SIZE, filename);

    if (num_of_chunks == -1) {
        cout << "getArrOfChunkNumbers is unsuccessful!";
        exit(EXIT_FAILURE);
    }

    //string private_ip = inet_ntoa(p2p_client_private_ip);
    string public_signal_ip_port =  get_signaller_public_ip_port();
    string str = "REQUEST 4 " + public_signal_ip_port + " " + DEFAULT_P2P_SERVER_PORT + " ";

    // TODO: Modify to include public IP
    for (auto chunk_no = 1; chunk_no <= num_of_chunks; chunk_no++) {
        str +=  filename + " " + to_string(chunk_no) + "|";
    }

    const char *buf = str.c_str();
    sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::quit(char *tracker_port) {

    this->connection(this->tracker_ip, tracker_port, true);
    string private_ip = inet_ntoa(p2p_client_private_ip);

    string str = "REQUEST 5 " + private_ip + " " + DEFAULT_P2P_SERVER_PORT;
    const char *buf = str.c_str();
    sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();

    printf("Goodbye!\n");
}

void p2p_client::inform_tracker_downloaded_chunk(char *tracker_port, string filename, string chunk_num) {

    this->connection(this->tracker_ip, tracker_port, true);

    string private_ip = inet_ntoa(p2p_client_private_ip);
    string str = "REQUEST 3 " + filename + " " + chunk_num + " " + private_ip + " " +
            DEFAULT_P2P_SERVER_PORT;

    const char *buf = str.c_str();
    sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::ask_updated_peer_list(char *tracker_port, string filename) {

    this->connection(this->tracker_ip, tracker_port, true);

    string str = "REQUEST 2 " + filename;
    const char *buf = str.c_str();
    sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);
    recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);

    peer_list.clear(); // destroys the current peer list

    string recv_str(recvbuf);
    parse_peer_list(peer_list, recvbuf);

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

int execute_user_option(p2p_client client) {

       int user_option;
       cin >> user_option;

       if (cin.fail()) {
           printf("Input is not an integer.\n");
           // The following is required to avoid cin fail infinite loop
           cin.clear();
           cin.ignore(numeric_limits<streamsize>::max(), '\n');
       }

       if (user_option < 1 || user_option > 7) {
           printf("No such option.\n");
           return -1;
       }

       switch (user_option) {
           case 1:
               cout << "Enter filename to download: ";
               cin >> filename;
               client.download_file(DEFAULT_TRACKER_PORT, filename);
               return 1;
           case 2:
               cout << "Enter filename to upload: ";
               cin >> filename;
               client.upload_file(DEFAULT_TRACKER_PORT, filename);
               return 2;
           case 3:
               client.query_list_of_files(DEFAULT_TRACKER_PORT);
               return 3;
           case 4:
               cout << "Enter filename: ";
               cin >> filename;
               client.query_file(DEFAULT_TRACKER_PORT, filename);
               return 4;
           default:
               client.quit(DEFAULT_TRACKER_PORT);
               return 5;
       }

}

bool p2p_client::setupSocketForSignallerServer(){
    string private_ip = inet_ntoa(p2p_client_private_ip);
    struct addrinfo *result = nullptr, hints{};
    int status;

    ZeroMemory(&result, sizeof(result));
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    string port = "6883";
    status = getaddrinfo(private_ip.c_str(), port.c_str(), &hints, &result);
    if (status != 0) {
        cout << "[ERROR]: " << status << " Unable to get address info for Port " << port << ".\n";
        return false;
    }

    SOCKET serv_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_sock == INVALID_SOCKET) {
        cout << "[ERROR]: " << WSAGetLastError() << " Unable to create Socket.\n";
        freeaddrinfo(result);
        return false;
    }

    if (setsockopt(serv_sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to set Socket options.\n";
        closesocket(serv_sock);
        return false;
    }

    if (::bind(serv_sock, result->ai_addr, (int) result->ai_addrlen) == SOCKET_ERROR) {
        cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket.\n";
        freeaddrinfo(result);
        closesocket(serv_sock);
        return false;
    }

    freeaddrinfo(result);
    signaller_sock = serv_sock;
    return true;
}


string p2p_client::get_signaller_public_ip_port() {
    string signal_server_ip = "18.136.118.72";
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

    Sleep(1);
    // TODO timeout
    int recv = recvfrom(signaller_sock,(char *)buf,MAXLINE, 0, nullptr, nullptr);
    buf[recv] = '\0';
    return string(buf);
}

int p2p_client::send_to_signal_public_ip(string public_signaller_ip_of_dest, char* data, int num_bytes_of_data_to_send){
    string signal_server_ip = "18.136.118.72";
    int signal_server_port = 6883;
    struct sockaddr_in servaddr;
    string dataToSend = public_signaller_ip_of_dest;
    dataToSend.append(" ");
    dataToSend.append(data, 0, num_bytes_of_data_to_send);
    // server
    memset(&servaddr, 0, sizeof(servaddr)); //sets all bytes of servaddr to 0
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, signal_server_ip.c_str(), &servaddr.sin_addr);
    servaddr.sin_port = htons(signal_server_port);

    // Send stun packet
    sendto(signaller_sock, dataToSend.c_str(), dataToSend.length(), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return 1; 
}

string p2p_client::connect_to_TURN_get_public_ip(SOCKET* sock){
    string signal_server_ip = "18.136.118.72";
    int bytes_recieved;  
    string send_data = "getPublic";
    char recv_data[1024];
    struct hostent *host;
    struct sockaddr_in server_addr;  

    host = gethostbyname(signal_server_ip.c_str());

    *sock = socket(AF_INET, SOCK_STREAM,0);
        if (connect_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(EXIT_FAILURE);
        }

    memset(&server_addr, 0, sizeof(server_addr)); //sets all bytes of servaddr to 0
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, signal_server_ip.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(6882);


    //connect to server at port 5000
    if (connect(*sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) 
    {
        return "";
    }
    send(*sock,send_data.c_str(),send_data.length(), 0);
        
    //get reply from server  
    bytes_recieved=recv(*sock,recv_data,1024,0);
    recv_data[bytes_recieved] = '\0';

    return string(recv_data);
}

int p2p_client::read_from_TURN_public_ip(SOCKET sock, char* data, int max_bytes_of_data_buffer_allocated){
    int bytes_received=recv(sock,data,max_bytes_of_data_buffer_allocated,0);
    return bytes_received;
}

//void p2p_client::testTURN(){
////    SOCKET temp;
////    string mystring = connect_to_TURN_get_public_ip(&temp);
////    cout << mystring << endl;
////    char buff[8000];
////    int readed = read_from_TURN_public_ip(temp,buff,8000);
////    buff[readed] = '\0';
////    cout << buff << endl;
//     setupSocketForSignallerServer();
//     string signalIP =  get_signaller_public_ip_port();
//     cout<<signalIP<<endl;
//     char buff[30];
//     char tempstr[] = "hello2shoe\0";
//     strcpy(buff,tempstr);
//     send_to_signal_public_ip("175.156.181.183:5071",buff,10);
//;}