template <typename T>
uint16_t Stun_Pkt_Handler::parse_request(T *pkt, int pkt_size, in_addr *public_ip) {

    auto *response_req_id = new unsigned char[REQUEST_ID_LEN];
    short cur_attr_type;
    int cur_attr_size;
    uint16_t port = 0;
    string public_ip_str;

    if(pkt_size < 20) {
        cout << "Stun response too short!\n";
        return 0;
    }
        // Check that response is 0x0101
    else if (*(u_short *)(pkt) != htons(0x0101)) {
        cout << "Stun response not of binding response type.\n";
        return 0;
    } else {

        cout << "Stun pkt is of binding response type!\n";

        // Extract Transaction ID
        memcpy(response_req_id, &pkt[8], REQUEST_ID_LEN);
        if(memcmp(response_req_id, request_id, REQUEST_ID_LEN) != 0) {
            cout << "Transaction IDs do not match\n";
            return 0;
        }

        // Iterate through attributes in response packet
        for(int i=RESPONSE_FIRST_ATTR_POS ; i<pkt_size; ) {

            // Extract length of attribute
            cur_attr_size = htons(*(u_short *)(&pkt[i+2]));
            cur_attr_type = htons(*(u_short *)(&pkt[i]));

            // If attribute is of 'xor-mapped-address'
            if(cur_attr_type == 0x0020) {
                // Check if IP address given is of IPv4
                if(pkt[i+5] != 1) {
                    cout << "Family of IP address in stun response is not IPv4\n";
                    return 0;
                    // Otherwise port and IP address is valid
                } else {
                    port = ntohs(*(uint16_t *)(&pkt[i+6]));
                    port ^= 0x2112; //XOR with magic cookie
                    // Maybe need to reverse the order, need to confirm
                    sprintf((char *)public_ip_str.c_str(), "%d.%d.%d.%d", pkt[i+8]^0x21, pkt[i+9]^0x12, pkt[i+10]^0xA4, pkt[i+11]^0x42);
                    inet_pton(AF_INET, public_ip_str.c_str(), public_ip);
                    break;
                }
            } else {
                i+=cur_attr_size;
                continue;
            }
        }
    }
    free(response_req_id);
    return port;
};
