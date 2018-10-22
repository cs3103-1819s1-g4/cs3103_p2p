//
// Created by Jarrett on 22/10/2018.
//

#include "p2p_client.h"
#include "../core/P2P_proto_packet.h"

// Winsock variables
SOCKET connect_socket;
WSAData wsa_data;
struct addrinfo *result = nullptr,
        *ptr = nullptr,
        hints;
int iresult;

using namespace std;

void p2p_client::display_menu() {

    printf("******P2P CLIENT******\n"
           "Enter options (1 to 5):\n"
           "1. Download a file\n"
           "2. Upload a file\n"
           "3. Query for a list of files available in tracker\n"
           "4. Query for a file available in tracker\n"
           "5. Quit\n"
           "Enter option: ");

}

int p2p_client::connect_to_tracker(char *tracker_ip, char *tracker_port) {

    iresult = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (iresult != 0) {
        printf("WSAStartup failed with error: %d\n", iresult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    iresult = getaddrinfo(tracker_ip, tracker_port, &hints, &result);
    if ( iresult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iresult);
        WSACleanup();
        return 1;
    }

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        connect_socket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (connect_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        break;
    }

    freeaddrinfo(result);

    if (connect_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    return 0;

}


void p2p_client::query_list_of_files(char *tracker_port) {

    this->connect_to_tracker(this->tracker_ip, tracker_port);

    string str = "REQUEST 6";
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    // TODO: Client needs to receive the list of files from tracker

    closesocket(connect_socket);
    WSACleanup();
}

void p2p_client::query_file(char *tracker_port, string filename) {

    this->connect_to_tracker(this->tracker_ip, tracker_port);

    string str = "REQUEST 7 ";
    str.append(filename);
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    // TODO: Client needs to receive the file from the tracker

    closesocket(connect_socket);
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
//               client.download_file(DEFAULT_TRACKER_PORT, DEFAULT_P2P_SERVER_PORT);
               return 1;
           case 2:
//               client.upload_file(DEFAULT_TRACKER_PORT);
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