//
// Created by Jarrett on 22/10/2018.
//
#ifndef P2P_CLIENT_H
#define P2P_CLIENT_H

#include <WS2tcpip.h>

#include "../core/core_functions.h"
#include "../core/P2P_proto_packet.h"


#define DEFAULT_TRACKER_PORT "80"
#define DEFAULT_P2P_SERVER_PORT "6881"

using namespace std;

const unsigned int MAX_CLIENT_BUFFER_LEN = 2048;

// Class p2p_client
class p2p_client {

private:
    bool online;
    char *tracker_ip;
    uint32_t client_ip;
    char *send_buffer;
    char *recv_buffer;
public:
    // Constructor
    explicit p2p_client(char *tracker_ip): online{true}, tracker_ip{tracker_ip} {

        /* To get client's private IP */
        IN_ADDR temp{};
        get_private_IP(temp);
        client_ip = ntohl(temp.s_addr);

        send_buffer = (char *)malloc(MAX_CLIENT_BUFFER_LEN);
        recv_buffer = (char *)malloc(MAX_CLIENT_BUFFER_LEN);

        std::cout << "Client running at " << client_ip << "\nTracker running at " << tracker_ip << "\n";
    }

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
        free(send_buffer);
        free(recv_buffer);
    }
};

int execute_user_option(p2p_client client);


#endif
