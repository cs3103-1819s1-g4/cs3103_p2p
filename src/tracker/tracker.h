//
// Created by Core on 22/10/2018.
//

#ifndef CS3103_P2P_TRACKER_H
#define CS3103_P2P_TRACKER_H

#include <WS2tcpip.h>
#include <winsock.h>
#include <string>
#include <list>
#include "../core/core_functions.h"
#include "../core/tracker_entries.h"


#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_TRACKER_PORT "80"
#define DEFAULT_P2P_SERVER_PORT "6881"
#define PACK_SIZE 65536  //Max length of buffer

using namespace std;


class tracker {
private:
    const char *port;
    in_addr server_ip;
    WSADATA wsa;
    vector<tracker_peer_list_entry*> peer_list;
    vector<tracker_file_list_entry*> file_list;

public:

    tracker(const char *port): port(port) {
        printf("\nInitialising Winsock...");
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            printf("Failed. Error Code : %d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        printf("Initialised.\n");


        get_private_IP(server_ip);

    };
    void init();
    string addEntry(string message);
    string addFile(string message);
    string query();
    string queryFile(string message);
    string generateList(string message);
    string updateIP(string message);
    string deleteIP(string message);
    void listen();
    void quit();

};


#endif //CS3103_P2P_TRACKER_H
