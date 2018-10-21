#ifndef CS3103_P2P_P2P_PROTO_PACKET_H
#define CS3103_P2P_P2P_PROTO_PACKET_H

#include <cstdint>
#include <string>
#include <vector>
#include <assert.h>
#include <iostream>
#include "tracker_entries.h"

#define REQUEST_TYPE_FIELD_LEN 7
#define RESPONSE_TYPE_FIELD_LEN 8

using namespace std;

static const char REQUEST_TYPE_FIELD[] = "REQUEST";
static const char RESPONSE_TYPE_FIELD[] = "RESPONSE";

/**
 * There are 2 default constructors for P2P request packet as 1 has variable length while the other has
 * fixed length.
 */
struct P2P_request_pkt {
private:
    char type[REQUEST_TYPE_FIELD_LEN];
    uint8_t flag;
    uint8_t file_name_len; // in bytes
    char *file_name;
    uint8_t chunk_no;
    uint32_t saddr;
public:
    /**
     * Constructor for creating P2P request packets for flag = 1, 2, 3, 4, 7
     * @param flag
     * @param file_name_len
     * @param file_name
     * @param chunk_no
     * @param saddr
     * @return
     */
    P2P_request_pkt* create_P2P_request_pkt(uint8_t flag, uint8_t file_name_len, char *file_name
            , uint8_t chunk_no, uint32_t saddr) {

        assert((flag > 0 && flag < 5) || flag == 7);   // check that flag is 1-7
        assert(file_name_len > 0);                 // Check that file length > 0

        P2P_request_pkt *packet_to_return = nullptr;
        size_t packet_size;

        try {
            packet_size = sizeof(P2P_request_pkt) + file_name_len;
            packet_to_return = (P2P_request_pkt *) malloc(packet_size);
            strcpy_s(packet_to_return->type, REQUEST_TYPE_FIELD_LEN, REQUEST_TYPE_FIELD);
            packet_to_return->flag = flag;
            packet_to_return->file_name_len = file_name_len;
            strcpy_s(packet_to_return->file_name, file_name_len, file_name);
            packet_to_return->chunk_no = chunk_no;
            packet_to_return->saddr = saddr;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating request packet failed\n";
        }
        return packet_to_return;
    };

    /**
     * Constructor for creating P2P request packets for flag = 5, 6
     * @param flag
     * @param saddr
     * @return
     */
    P2P_request_pkt* create_P2P_request_pkt(uint8_t flag, uint32_t saddr) {

        assert(flag == 5 || flag == 6);
        P2P_request_pkt *packet_to_return = nullptr;

        try {
            packet_to_return = (P2P_request_pkt *) malloc(sizeof(P2P_request_pkt));
            strcpy_s(packet_to_return->type, REQUEST_TYPE_FIELD_LEN, REQUEST_TYPE_FIELD);
            packet_to_return->flag = flag;
            packet_to_return->file_name_len = 0;
            packet_to_return->file_name = nullptr;
            packet_to_return->chunk_no = NULL;
            packet_to_return->saddr = saddr;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating request packet failed\n";
        }

        return packet_to_return;
    };
};

/**
 * Have to be declare using the following: P2P_response_pkt<T> name
 * @tparam T
 */
template <typename T>
struct P2P_response_pkt {
private:
    char type[RESPONSE_TYPE_FIELD_LEN];
    uint8_t list_len;
    vector<T> entry_list;
public:
    P2P_response_pkt create_P2P_response_pkt(uint8_t list_len, vector<T> list) {
        assert(list_len > 0);

        P2P_response_pkt packet_to_return;

        strcpy_s(packet_to_return->type, RESPONSE_TYPE_FIELD_LEN, RESPONSE_TYPE_FIELD);
        packet_to_return->list_len = list_len;

        for(int i=0 ; i<list_len ; i++) {
            packet_to_return->entry_list.push_back(list[i]);
        }
        return packet_to_return;
    };
};
#endif //CS3103_P2P_P2P_PROTO_PACKET_H
