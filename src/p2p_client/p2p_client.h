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
#define PATH_TO_STORAGE_DIRECTORY "..\\to_upload"

using namespace std;

// Class p2p_client
class p2p_client {

private:
    bool online;
    const char *tracker_ip;
    P2P_Server *p2p_server;
    Storage *p2p_client_storage;
public:
    // Constructor
    explicit p2p_client(const char *tracker_ip) {
        online = true;
        tracker_ip = tracker_ip;
        p2p_client_storage = new Storage(PATH_TO_STORAGE_DIRECTORY);
        p2p_server = new P2P_Server(p2p_client_storage);
    };

    void display_menu();

    void connection(const char *ip_addr, const char *port_num, bool is_tracker);
    void inform_tracker_downloaded_chunk(char *tracker_port, string filename, string chunk_num);

    // These functions are ONLY for p2p_client and tracker communication
    void query_list_of_files(char *tracker_port);
    void query_file(char *tracker_port, string filename);

    // These functions are involved p2p_client and p2p_server communication
    void download_file(char *tracker_port, string filename);
    void upload_file(char *tracker_port, string filename);

    void quit();

    bool start_p2p_server_thread();

    //Destructor
    ~p2p_client() {
        online = false;
        tracker_ip = nullptr;
        p2p_server = nullptr;
    }
};

int execute_user_option(p2p_client client);

#endif
