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

    // These functions are involved p2p_client and TURN and Signaller communication
    bool setupSocketForSignallerServer();
    string get_signaller_public_ip_port(); //"192.168.1.1:5000"
    // send data to signaller to relay to dest, returns 1 if success
    int send_to_signal_public_ip(string public_signaller_ip_of_dest, char* data, int num_bytes_of_data_to_send);
    // connect to TURN with socket (pass in a blank socket socket)
    string connect_to_TURN_get_public_ip(SOCKET sock);
    // returns number of bytes read from TURN socket and bytes stored in to buffer
    int read_from_TURN_public_ip(SOCKET sock,char* data, int max_bytes_of_data_buffer_allocated);


    void quit(char *tracker_port);

    //Destructor
    ~p2p_client() {
        online = false;
        tracker_ip = nullptr;
    }
};

int execute_user_option(p2p_client client);

#endif
