#ifndef CS3103_P2P_P2P_PROTO_PACKET_H
#define CS3103_P2P_P2P_PROTO_PACKET_H

#include <vector>
#include "tracker_entries.h"

#define REQUEST_TYPE_FIELD_LEN 8
#define RESPONSE_TYPE_FIELD_LEN 9

using namespace std;

static const char REQUEST_TYPE_FIELD[] = "REQUEST\0";
static const char RESPONSE_TYPE_FIELD[] = "RESPONSE\0";
const int FILE_NAME_LEN = 256;

/**
 * There are 2 constructors for P2P request packet
 */
struct P2P_request_pkt {
public:
    char type[REQUEST_TYPE_FIELD_LEN];
    uint8_t flag;
    uint8_t file_name_len; // in bytes
    char file_name[FILE_NAME_LEN];
    uint8_t chunk_no;
    uint32_t saddr;

    /**
     * Constructor for creating P2P request packets for flag = 1, 2, 3, 4, 7
     * 1 - Download file from swarm. File name must be filled
     * 2 - Ask tracker for updated list, File name must be filled
     * 3 - Inform tracker a certain chunk has been successfully downloaded. File name, chunk no must
     * be filled
     * 4 - Upload a new file. File name, chunk no, IP address must be filled
     * 7 - Query for a specified file. File name must be filled
     * @param flag
     * @param file_name_len
     * @param file_name
     * @param chunk_no
     * @param saddr
     * @return
     */
    P2P_request_pkt(uint8_t flag, uint8_t file_name_len, char *file_name
            , uint8_t chunk_no, uint32_t saddr) {

        assert((flag > 0 && flag < 5) || flag == 7);
        assert(file_name_len > 0 && file_name_len < FILE_NAME_LEN); // leave a space for '\0'

        try {
            strcpy_s(this->type, REQUEST_TYPE_FIELD_LEN, REQUEST_TYPE_FIELD);
            this->flag = flag;
            this->file_name_len = file_name_len;
            strcpy_s(this->file_name, this->file_name_len, file_name);
            this->file_name[file_name_len] = '\0';
            this->chunk_no = chunk_no;
            this->saddr = saddr;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating request packet failed\n";
        }
    };

    /**
    * Constructor for creating P2P request packets for flag = 5, 6
    * 5 - Exit from swarm. IP address must be filled
    * 6 - Query the tracker for list of files. None filled
    * @param flag
    * @param saddr
    * @return
    */
    P2P_request_pkt(uint8_t flag, uint32_t saddr) {

        assert(flag == 5 || flag == 6);
        try {
            strcpy_s(this->type, REQUEST_TYPE_FIELD_LEN, REQUEST_TYPE_FIELD);
            this->flag = flag;
            this->file_name_len = 0;
            memset(file_name, '\0', sizeof(file_name));
            this->chunk_no = NULL;
            this->saddr = saddr;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating request packet failed\n";
        }
    };

};


/**
 *
 */
template <typename T>
struct p2p_response_pkt {
public:
    char type[RESPONSE_TYPE_FIELD_LEN];
    uint8_t list_len;
    vector<T> entry_list;

    p2p_response_pkt(uint8_t list_len, vector<T> list) {
        assert(list_len > 0);

        strcpy_s(this->type, RESPONSE_TYPE_FIELD_LEN, RESPONSE_TYPE_FIELD);
        this->list_len = list_len;

        for(int i=0 ; i<list_len ; i++) {
            this->entry_list.push_back(list[i]);
        }
    };
};
#endif //CS3103_P2P_P2P_PROTO_PACKET_H
