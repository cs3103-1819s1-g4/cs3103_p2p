//
// Created by Jarrett on 22/10/2018.
//
#ifndef P2P_CLIENT_H
#define P2P_CLIENT_H

#include <WS2tcpip.h>

#include "../core/core_functions.h"

#define DEFAULT_TRACKER_PORT "80"
#define DEFAULT_P2P_SERVER_PORT "6881"

using namespace std;

// Class p2p_client
class p2p_client {

private:
    bool online;
    const char *tracker_ip;
    uint32_t client_ip;
public:
    // Constructor
    explicit p2p_client(const char *tracker_ip): online{true}, tracker_ip{tracker_ip} {

        /*TODO: To get client's private IP
        in_addr temp{};
        get_private_IP(temp);
        client_ip = temp.s_addr;


        std::cout << "Client running at " << inet_ntoa(temp.s_addr) << "\nTracker running at "
         << tracker_ip << "\n";*/
    }

    void display_menu();

    // These functions are ONLY for p2p_client and tracker communication
    void connect_to_tracker(const char *tracker_ip, char *tracker_port);
    void query_list_of_files(char *tracker_port);
    void query_file(char *tracker_port, string filename);

    // These functions are involved p2p_client and p2p_server communication
//    void download_file(char *tracker_port, char *p2p_server_port);
    void upload_file(char *tracker_port, string filename);

    void quit();

    //Destructor
    ~p2p_client() {
        online = false;
        tracker_ip = nullptr;
    }
};

int execute_user_option(p2p_client client);


#endif
