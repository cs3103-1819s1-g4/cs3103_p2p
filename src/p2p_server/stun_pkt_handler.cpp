#include "stun_pkt_handler.h"

Stun_Pkt_Handler::Stun_Pkt_Handler() {

    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&h_provider, BCRYPT_RNG_ALGORITHM,
                                                    nullptr, 0))) {
        cout << "[ERROR]" << GetLastError() << "\t configuring Bcrypt handle failed\n";
        exit(1);
    }
    if(!BCRYPT_SUCCESS(BCryptGenRandom(h_provider, (PUCHAR) request_id, sizeof(request_id), 0))) {
        cout << "[ERROR]" << GetLastError() << "\t configuring Bcrypt generate failed\n";
        exit(1);
    }
    BCryptCloseAlgorithmProvider(h_provider, 0);
}

void Stun_Pkt_Handler::create_first_bind_pkt(unsigned char *binding_req_pkt) {

    *(short *)(&binding_req_pkt[0]) = (u_short) htons(0x0001);    // 0x0001 - Binding Request
    *(short *)(&binding_req_pkt[2]) = (u_short) htons(0x0000);    // Length of pkt without header, last 2 bits always '0's
    *(int *)(&binding_req_pkt[4]) = (u_long) htonl(0x2112A442);  // Magic cookie of fixed value
    memcpy(binding_req_pkt + 8, request_id, sizeof(request_id));    //Transaction ID, ignore network order
}
