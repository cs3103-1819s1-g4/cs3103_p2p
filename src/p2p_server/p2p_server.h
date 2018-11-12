// TODO: in p2p_server, switch to using p2p_request.h, instead of p2p_request_response_pakcet.h

#ifndef CS3103_P2P_P2P_SERVER_H
#define CS3103_P2P_P2P_SERVER_H

#include "../core/core_functions.h"
#include "../core/p2p_request_response_packet.h"
#include "../core/storage.h"
#include "stun_pkt_handler.h"
#include <sstream>

using namespace std;

const char CHUNK_NOT_FOUND_ERROR[] = "CHUNK NOT FOUND";
const int MAX_BUFFER_LEN = 65536;

class P2P_Server {
private:
    bool online;
    SOCKET listen_sock;
    SOCKET signaller_sock;
    in_addr p2p_server_private_ip;
    in_addr p2p_server_public_ip;
    uint16_t p2p_server_public_port;
    char *recv_buffer;
    char *send_buffer;
    char *chunk_buffer;
    Storage *storage;
    vector< pair<string, uint16_t> > STUN_SERV_VECTOR {make_pair("stun.l.google.com", 19305),
                                                      make_pair("stun1.l.google.com", 19305),
                                                      make_pair("stun2.l.google.com", 19305),
                                                      make_pair("stun3.l.google.com", 19305),
                                                      make_pair("stun4.l.google.com", 19305)};
public:
    /**
     * Constructor for P2P server
     */
    explicit P2P_Server(Storage *p2p_client_storage) : listen_sock(INVALID_SOCKET), online(false), storage(p2p_client_storage){

        WSADATA wsock;
        int status = WSAStartup(MAKEWORD(2,2),&wsock);

        if ( status != 0)
            cout << "[ERROR]: " << status << " Unable to start Winsock.\n";
        else
            online = true;

        get_private_IP(p2p_server_private_ip);

        recv_buffer = (char *)malloc(MAX_BUFFER_LEN);
        send_buffer = (char *)malloc(MAX_BUFFER_LEN);
        chunk_buffer = (char *)malloc(MAX_BUFFER_LEN);
    };

    /**
     * destructor for P2P server
     */
    ~P2P_Server() {
        stop();
        if (online)
            WSACleanup();
        if (recv_buffer != nullptr)
            recv_buffer = nullptr;
        if (send_buffer != nullptr)
            send_buffer = nullptr;
        if (chunk_buffer != nullptr)
            chunk_buffer = nullptr;
    };

    bool start(const char *port);
    void stop();
    bool listen();

    /**
     * Process datagram in recv_buffer and reply peer.
     * @param client_addr
     * @param sin_size
     * @return true as long as a reply is sent (even if chunk retrieval is unsuccessful). false otherwise.
     */
    bool process_request(sockaddr_in client_addr, int sin_size);
    pair<string, string> parse_packet();
    bool get_public_ip_stun();
    bool get_public_ip_stun2(char * return_ip_port, char * default_private_server_port);
    int stun_xor_addr(const char * stun_server_ip,short stun_server_port,short local_port,char * return_ip_port);

    // These functions are involved p2p_server and TURN and Signaller communication
    bool setupSocketForSignallerServer();
    string get_signaller_public_ip_port(); //"192.168.1.1:5000"
    // returns number of bytes read from signaller
    int read_from_signal_public_ip(char* data, int max_bytes_of_data_buffer_allocated);
    // send data to TURN to relay to dest, returns 1 if success
    int send_to_TURN_public_ip(string public_TURN_ip_of_dest, char* data, int num_bytes_of_data_to_send);

};

#endif //CS3103_P2P_P2P_SERVER_H
