//
// Created by Jarrett on 22/10/2018.
//
#ifndef P2P_CLIENT_H
#define P2P_CLIENT_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>

#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_TRACKER_PORT "80"
#define DEFAULT_P2P_SERVER_PORT "6881"

using namespace std;

// Class p2p_client
class p2p_client {

private:
    bool online;
    char *tracker_ip;
public:
    // Constructor
    p2p_client(char *tracker_ip): online{true}, tracker_ip{tracker_ip} {}

    void display_menu();

    // These functions are ONLY for p2p_client and tracker communication
    int connect_to_tracker(char *tracker_ip, char *tracker_port);
    void query_list_of_files(char *tracker_port);
    void query_file(char *tracker_port, string filename);

    // These functions are involved in p2p_client and p2p_server communication
//    void download_file(char *tracker_port, char *p2p_server_port);
//    void upload_file(char *tracker_port);

    void quit();

    //Destructor
    ~p2p_client() {
        online = false;
        tracker_ip = nullptr;
    }
};

int execute_user_option(p2p_client client);


#endif
