//
// Created by Jarrett on 22/10/2018.
//

#include "p2p_client.h"
#include <iostream>
#include <fstream>
#include "../core/storage.h"
#include "../core/p2p_request.h"
#include <map>

#define MAX_BUFFER_SIZE 65536

using namespace std;

// Winsock variables
SOCKET connect_socket;
WSAData wsa_data;
struct addrinfo *result = nullptr,
        *ptr = nullptr,
        hints;
int iresult;

char recvbuf[MAX_BUFFER_SIZE];

map <int, string> peer_list;

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

void p2p_client::connection(const char *tracker_ip, char *tracker_port) {

    iresult = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (iresult != 0) {
        printf("WSAStartup failed with error: %d\n", iresult);
        exit(EXIT_FAILURE);
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    iresult = getaddrinfo(tracker_ip, tracker_port, &hints, &result);
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

    if (connect_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

}

void p2p_client::download_file(char *tracker_port, string filename) {

    this->connection(this->tracker_ip, tracker_port);

    string str = "REQUEST 1 " + filename;
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);
    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);

    // TODO: Save the list of peers into an array
//    string recv_str(recvbuf);
//    parse_peer_list(peer_list, recvbuf);

    closesocket(connect_socket);
    WSACleanup();

    // TODO: Connect to p2p_server

//    int random_server = choose_random_server(peer_list);
//
//    // p2p_server_ip, p2p_server_port and chunk_number
//
//    this->connection(p2p_server_IP, p2p_server_port);
//
//    string str = "DOWNLOAD " + filename + " " + chunk_number;
//
//    closesocket(connect_socket);
//    WSACleanup();

}

void p2p_client::query_list_of_files(char *tracker_port) {

    this->connection(this->tracker_ip, tracker_port);

    string str = "REQUEST 6";
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);
    string recv_str(recvbuf);
    cout << recv_str;

    // TODO: Maybe I have to parse the output to make it look nicer.

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::query_file(char *tracker_port, string filename) {

    assert(filename.length() < 256);

    this->connection(this->tracker_ip, tracker_port);

    string str = "REQUEST 7 " + filename;
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);
    string recv_str(recvbuf);
    cout << recv_str;

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::upload_file(char *tracker_port, string filename) {

    this->connection(this->tracker_ip, tracker_port);

    Storage storage("..\\download");
    int chunk_no_buffer[MAX_BUFFER_SIZE];
    int num_of_chunks = storage.getArrOfChunkNumbers(chunk_no_buffer, MAX_BUFFER_SIZE, filename);

    if (num_of_chunks == -1) {
        cout << "getArrOfChunkNumbers is unsuccessful!";
        exit(EXIT_FAILURE);
    }

    string str = "REQUEST 4 ";

    for (auto chunk_no = 1; chunk_no <= num_of_chunks; chunk_no++) {
        str = str + filename + " " + to_string(chunk_no) + "|";
    }

    const char *buf = str.c_str();
    sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::quit() {
    printf("Goodbye!\n");
}

int execute_user_option(p2p_client client) {

       int user_option;
       string filename;
       cin >> user_option;

       if (cin.fail()) {
           printf("Input is not an integer.\n");
           return -1;
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
               client.quit();
               return 5;
       }

}
