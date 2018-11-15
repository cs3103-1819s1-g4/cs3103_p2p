// TODO: in p2p_server, switch to using p2p_request.h, instead of p2p_request_response_pakcet.h

#ifndef CS3103_P2P_P2P_SERVER_H
#define CS3103_P2P_P2P_SERVER_H

#include "../core/core_functions.h"
#include "../core/p2p_request_response_packet.h"
#include "../core/storage.h"
#include <WS2tcpip.h>
#include <sstream>

using namespace std;

const char CHUNK_NOT_FOUND_ERROR[] = "CHUNK NOT FOUND";
const int MAX_BUFFER_LEN = 65546;
const int MAX_CONNECTIONS = 10;

class P2P_Server {
private:
    bool online;
    SOCKET listen_sock;
    string *signal_public_ip;
    SOCKET signal_sock;
    in_addr p2p_server_private_ip;
    Storage *storage;
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
    };

    /**
     * destructor for P2P server
     */
    ~P2P_Server() {
        stop();
        if (online)
            WSACleanup();
    };

    bool start(const char *port);
    void stop();
    bool listen(string& signal_public_ip);

    bool process_request(string request);
    tuple<string, string, string> parse_packet(char *recv_buffer);

    // These functions are involved p2p_server and TURN and Signaller communication
    bool setupSocketForSignallerServer();
    void send_signaller_public_ip_port(); //"192.168.1.1:5000"
    // returns number of bytes read from signaller
    int read_from_signal_public_ip(char* data, int max_bytes_of_data_buffer_allocated);
    // send data to TURN to relay to dest, returns 1 if success
    int send_to_TURN_public_ip(string public_TURN_ip_of_dest, char* data, int num_bytes_of_data_to_send);
    // an example on how to use the functions
    //void testTURN();
    bool keep_alive_udp();

};

#endif //CS3103_P2P_P2P_SERVER_H
