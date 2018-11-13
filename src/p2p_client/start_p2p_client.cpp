//
// Created by Jarrett on 22/10/2018.
//

// start_p2p_client starts the P2P client AND the P2P server

#include <iostream>
#include <cstdio>
#include <thread>
#include <functional>
#include "p2p_client.h"

#define PATH_TO_STORAGE_DIRECTORY "..\\to_upload"

#pragma comment(lib, "Bcrypt")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

using namespace std;

void start_p2p_server_thread(Storage& storage);
void start_p2p_client_thread(Storage& storage);

int main() {

    Storage storage(PATH_TO_STORAGE_DIRECTORY);

    thread p2p_server_thread(start_p2p_server_thread, std::ref(storage));
    Sleep(1000);

    thread p2p_client_thread(start_p2p_client_thread, std::ref(storage));

    p2p_server_thread.join();
    p2p_client_thread.join();

    return 0;
}

void start_p2p_client_thread(Storage& storage) {

    string tracker_ip_string;
    int user_option;

    cout << "\n#################### Welcome to P2P client ####################\n"
            "\nEnter Tracker's IP address: ";
    cin >> tracker_ip_string;

    const char *tracker_ip = tracker_ip_string.c_str();
    p2p_client client(tracker_ip, &storage);

    do {
        client.display_menu();
        user_option = execute_user_option(client);
    } while (user_option != 5);
}

void start_p2p_server_thread(Storage& storage) {

    P2P_Server p2p_server(&storage);

    if (!p2p_server.start(DEFAULT_P2P_SERVER_PORT)) {
        cout << "Failed to start server";
    } else {
        p2p_server.listen();
    }
}
