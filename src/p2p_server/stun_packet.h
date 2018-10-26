#ifndef CS3103_P2P_STUN_PACKET_H
#define CS3103_P2P_STUN_PACKET_H

#include <bcrypt.h>
#include <cstdio>
#include <iostream>
#include <windows.h>

#pragma comment(lib, "Bcrypt")

using namespace std;

const unsigned int STUN_PKT_LEN = 20;
const int REQUEST_ID_LEN = 12;
const int RESPONSE_FIRST_ATTR_POS = 20;
const unsigned char ATTR_TYPE_MAPPED_ADDR[] = {0x00,0x01};

class STUN_Packet {
private:
    BCRYPT_ALG_HANDLE h_provider;
    unsigned char request_id[REQUEST_ID_LEN];
public:
    STUN_Packet() {
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

    void create_first_bind_pkt(unsigned char *binding_req_pkt);

    // Consider using sockaddr_in for ip
    template <typename T, size_t n>
    bool parse_request(T (&packet)[n], in_addr *public_ip);
};
#endif //CS3103_P2P_STUN_PACKET_H
