//
// Created by Jarrett on 22/10/2018.
//

// start_p2p_client starts the P2P client

#include <iostream>
#include <cstdio>
#include <thread>
#include <functional>
#include "p2p_client.h"

#pragma comment(lib, "Bcrypt")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

using namespace std;

int main() {

    string tracker_ip_string;

    cout << "#################### Welcome to P2P client ####################\n"
                 "\nEnter Tracker's IP address: ";

    const char *tracker_ip = tracker_ip_string.c_str();

    int user_option;
    p2p_client client(tracker_ip);

    thread p2p_server_thread(&p2p_client::start_p2p_server_thread, client);
    p2p_server_thread.detach();

    Sleep(2000);

    do {

        client.display_menu();
        user_option = execute_user_option(client);
    } while (user_option != 5);

    return 0;
}
