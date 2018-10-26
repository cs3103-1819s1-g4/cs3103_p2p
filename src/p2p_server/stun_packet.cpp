#include "stun_packet.h"

void STUN_Packet::create_first_bind_pkt(unsigned char *binding_req_pkt) {
    binding_req_pkt = new unsigned char[STUN_PKT_LEN];

    *(short *)(&binding_req_pkt[0]) = htons(0x0001);    // 0x0001 - Binding Request
    *(short *)(&binding_req_pkt[2]) = htons(0x0000);    // Length of packet without header, last 2 bits always '0's
    *(int *)(&binding_req_pkt[4]) = htonl(0x2112A442);  // Magic cookie of fixed value
    memcpy(binding_req_pkt + 8, request_id, sizeof(request_id));    //Transaction ID, ignore network order
}

// Consider using sockaddr_in for ip
template <typename T, size_t n>
int STUN_Packet::parse_request(T (&packet)[n], in_addr *public_ip) {

    auto *response_request_id = new unsigned char[REQUEST_ID_LEN];
    auto *response_attr_type = new unsigned char[RESPONSE_ATTR_TYPE_LEN];
    int response_size, attr_size;

    if(sizeof(T) * n < 20) {
        cout << "Stun response too short!\n";
        return 1;
    }
        // Check that response is 0x0101
    else if (packet[0] != 1 || packet[1] != 1) {
        cout << "Stun response not of type binding response.\n";
        return 1;
    } else {

        // Extract Transaction ID
        memcpy(response_request_id, packet + 8, REQUEST_ID_LEN);
        if(memcmp(response_request_id, request_id, REQUEST_ID_LEN) != 0) {
            cout << "Transaction IDs do not match\n";
            return 1;
        }
        // Extract size of response packet without header
        response_size = packet[2] * 256 + packet[3];
        if(response_size < 4) {
            cout << "Value field too small to have mapped address attribute\n";
            return 1;
        }

        // Iterate through attributes in response packet
        for(int i=RESPONSE_FIRST_ATTR_POS ; i<response_size + RESPONSE_FIRST_ATTR_POS; ) {

            // Extract length of attribute
            attr_size = packet[i+2]*256 + packet[i+3];

            //Extract attribute type
            memcpy(response_attr_type, packet[i], sizeof(ATTR_TYPE_MAPPED_ADDR));
            // If attribute is of 'mapped address'
            if(memcmp(response_attr_type, ATTR_TYPE_MAPPED_ADDR, sizeof(ATTR_TYPE_MAPPED_ADDR)) == 0) {
                if(attr_size < 8) {
                    cout << "Mapped address attribute is too short\n";
                    return 1;

                    // Check if IP address given is of IPv4
                } else if(packet[i+5] != 1) {
                    cout << "Family of IP address given is not IPv4\n"
                    return 1;

                    // Otherwise port and IP address is valid
                } else {
                    port = packet[i+6] * 256 + packet[i+7];
                    uint8_t *addr_ptr = packet[i+8];
                    memcpy((void *)public_ip, addr_ptr, sizeof(in_addr));
                    free(addr_ptr);
                    break;
                }
            } else {
                i+=attr_size;
                continue;
            }
        }
    }
    free(response_request_id);
    free(response_attr_type);
    return port;
};

