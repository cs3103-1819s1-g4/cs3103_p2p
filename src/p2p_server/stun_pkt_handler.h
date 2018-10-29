#ifndef CS3103_P2P_STUN_PKT_HANDLER_H
#define CS3103_P2P_STUN_PKT_HANDLER_H

#include <cstdio>
#include <iostream>
#include <WS2tcpip.h>
#include <bcrypt.h>

using namespace std;

const unsigned int STUN_PKT_LEN = 20;
const int REQUEST_ID_LEN = 12;
const int RESPONSE_FIRST_ATTR_POS = 20;

// TODO: Rebinding after public ip expire

class Stun_Pkt_Handler {
private:
    BCRYPT_ALG_HANDLE h_provider;
    unsigned char request_id[REQUEST_ID_LEN];
public:
    Stun_Pkt_Handler();

    /**
     * Create the first bind request packet
     * @param binding_req_pkt must be pointing to array of 20 bytes in length
     */
    void create_first_bind_pkt(unsigned char *binding_req_pkt);

    /**
     * Parse STUN server responses
     * @tparam T To allow char data type to be passed into the function. Should be unsigned char by right.
     * @param packet Packet to be parsed
     * @param pkt_size Size of packet to be parsed
     * @param public_ip Returns public IP if parsing is successful
     * @return 0 if parsing failed. Returns public port number otherwise
     */
    template <typename T>
    uint16_t parse_request(T *packet, int pkt_size, in_addr *public_ip);
};

#include "stun_pkt_handler.tpp"
#endif //CS3103_P2P_STUN_PKT_HANDLER_H
