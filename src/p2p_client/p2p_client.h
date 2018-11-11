//
// Created by Jarrett on 22/10/2018.
//
#ifndef P2P_CLIENT_H
#define P2P_CLIENT_H

#include <WS2tcpip.h>

#include "../core/core_functions.h"
#include "../p2p_server/p2p_server.h"

#define DEFAULT_TRACKER_PORT "80"
#define DEFAULT_P2P_SERVER_PORT "6881" // this is not used when NAT is involved

using namespace std;

// Class p2p_client
class p2p_client {

private:
    bool online;
    const char *tracker_ip;
    in_addr p2p_client_private_ip;
    Storage *p2p_client_storage;
public:
    // Constructor
    explicit p2p_client(const char *tracker_ip, Storage *storage) {
        online = true;
        this->tracker_ip = tracker_ip;
        get_private_IP(p2p_client_private_ip);
        p2p_client_storage = storage;
    };

    void display_menu();

    int connection(const char *ip_addr, const char *port_num, bool is_tracker);
    void inform_tracker_downloaded_chunk(char *tracker_port, string filename, string chunk_num);
    void ask_updated_peer_list(char *tracker_port, string filename);

    // These functions are ONLY for p2p_client and tracker communication
    void query_list_of_files(char *tracker_port);
    void query_file(char *tracker_port, string filename);

    // These functions are involved p2p_client and p2p_server communication
    void download_file(char *tracker_port, string filename);
    void upload_file(char *tracker_port, string filename);

    void quit(char *tracker_port);

    //Destructor
    ~p2p_client() {
        online = false;
        tracker_ip = nullptr;
    }
};

int execute_user_option(p2p_client client);

#endif
