//
// Created by Jarrett on 22/10/2018.
//

// start_p2p_client starts the P2P client
// Run the p2p_client by using "p2p_client <Tracker's IP>"

#include <iostream>
#include <cstdio>
#include "p2p_client.h"

using namespace std;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Please supply the IP address of tracker\n");
        return 1;
    }

    char *tracker_ip = argv[1];

    int user_option;
    p2p_client client(tracker_ip);

    do {
        client.display_menu();
        user_option = execute_user_option(client);
    } while (user_option != 5);


    return 0;
}
