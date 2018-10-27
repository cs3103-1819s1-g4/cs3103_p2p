#ifndef CS3103_P2P_STUN_PKT_HANDLER_H
#define CS3103_P2P_STUN_PKT_HANDLER_H

#include <bcrypt.h>
#include <cstdio>
#include <iostream>
#include <Windows.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Bcrypt")

using namespace std;

const unsigned int STUN_PKT_LEN = 20;
const int REQUEST_ID_LEN = 12;
const int RESPONSE_FIRST_ATTR_POS = 20;
const unsigned char ATTR_TYPE_MAPPED_ADDR[] = {0x00,0x01};

// TODO: Rebinding after public ip expire
// TODO: Find another crypto library, Bcrypt dependencies on latest windows SDK is troublesome

class Stun_Pkt_Handler {
private:
    BCRYPT_ALG_HANDLE h_provider;
    unsigned char request_id[REQUEST_ID_LEN];
public:
    Stun_Pkt_Handler() {
        if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&h_provider, BCRYPT_RNG_ALGORITHM,
                                                        NULL, 0))) {
            cout << "[ERROR]" << GetLastError() << "\t configuring Bcrypt handle failed\n";
            exit(1);
        }
        if(!BCRYPT_SUCCESS(BCryptGenRandom(h_provider, (PUCHAR) request_id, sizeof(request_id), 0))) {
            cout << "[ERROR]" << GetLastError() << "\t configuring Bcrypt generate failed\n";
            exit(1);
        }
        // BCryptCloseAlgorithmProvider(h_provider, 0);
    }

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


#endif //CS3103_P2P_STUN_PKT_HANDLER_H
