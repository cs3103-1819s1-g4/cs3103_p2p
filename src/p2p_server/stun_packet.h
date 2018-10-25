#ifndef CS3103_P2P_STUN_PACKET_H
#define CS3103_P2P_STUN_PACKET_H

#include <bcrypt.h>
#include <cstdio>
#include <iostream>
#include <windows.h>

#pragma comment(lib, "Bcrypt")

using namespace std;

const unsigned int STUN_PKT_LEN = 32;
const int REQUEST_ID_LEN = 16;
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

    void create_packet(unsigned char *packet) {
        packet = new unsigned char[STUN_PKT_LEN] {
            //header
            0x00,0x01,  // 0x0001 = Binding request
            0x00,0x08,  // length of packet without header, last 2 bits of this field is always '0's
            0x21,0x12,0xA4,0x42,    // Magic cookie of fixed value
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    // buffer transaction ID
            0x00,0x03,  // Type: change request
            0x00,0x04,  // Length of value(attribute)
            0x00,0x00,0x00,0x00 // IP address
        };

        // Set Transaction ID
        memcpy(packet + 8, request_id, sizeof(request_id));
    }

    // Consider using sockaddr_in for ip
    template <typename T, size_t n>
    bool parse_request(T (&packet)[n], int *port, in_addr *public_ip) {

        auto *response_request_id = new unsigned char[REQUEST_ID_LEN];
        auto *response_attr_type = new unsigned char[RESPONSE_ATTR_TYPE_LEN];
        int response_size, attr_size;

        if(sizeof(T) * n < 20) {
            cout << "Stun response too short!\n";
            return false;
        }
        // Check that response is 0x0101
        else if (packet[0] != 1 || packet[1] != 1) {
            cout << "Stun response not of type binding response.\n";
            return false;
        } else {

            // Extract Transaction ID
            memcpy(response_request_id, packet + 8, REQUEST_ID_LEN);
            if(memcmp(response_request_id, request_id, REQUEST_ID_LEN) != 0) {
                cout << "Transaction IDs do not match\n";
                return false;
            }
            // Extract size of response packet without header
            response_size = packet[2] * 256 + packet[3];
            if(response_size < 4) {
                cout << "Value field too small to have mapped address attribute\n";
                return false;
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
                        return false;

                        // Check if IP address given is of IPv4
                    } else if(packet[i+5] != 1) {
                        cout << "Family of IP address given is not IPv4\n"
                        return false;

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
        return true;
    };
};
#endif //CS3103_P2P_STUN_PACKET_H
