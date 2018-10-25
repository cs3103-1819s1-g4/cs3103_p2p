//
// Created by Core on 22/10/2018.
//

#ifndef CS3103_P2P_TRACKER_H
#define CS3103_P2P_TRACKER_H

#include <WS2tcpip.h>
#include <winsock.h>
#include <string>
#include "../core/core_functions.h"
#include "../core/tracker_entries.h"

#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_TRACKER_PORT "80"
#define DEFAULT_P2P_SERVER_PORT "6881"
#define PACK_SIZE 2048  //Max length of buffer

using namespace std;


class tracker {
private:
    const char *port;
    IN_ADDR server_ip;
    WSADATA wsa;
    vector<tracker_peer_list*> peer_list;
    vector<tracker_file_list*> file_list;

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
    string addEntry(string message,string ip,int port);
    string addFile(string message);
    string query(string message);
    string generateList(string message);
    string updateIP(string message);
    string deleteIP(string message);
    tracker_peer_list* createDummyEntry();
    void listen();
    void quit();

};


#endif //CS3103_P2P_TRACKER_H
