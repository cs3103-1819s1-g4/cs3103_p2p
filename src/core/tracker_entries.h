#ifndef CS3103_P2P_TRACKER_ENTRIES_H
#define CS3103_P2P_TRACKER_ENTRIES_H

#include <assert.h>
#include <cstdint>
#include <iostream>

class tracker_peer_list {

private:

    uint8_t file_name_len;
    char *file_name;
    uint8_t chunk_no;
    uint32_t public_IP;
    uint16_t port_no;

public:
    /**
     * constructor for client entry in tracker
     * @param filename_len
     * @param file_name
     * @param chunk_no
     * @param public_IP
     * @param port_no
     */
    tracker_peer_list(uint8_t file_name_len, char *file_name, uint8_t chunk_no, uint32_t public_IP
            , uint16_t port_no) {

        assert(file_name_len > 0 && file_name_len < 256);

        try {
            this->file_name_len = (uint8_t) (file_name_len + 1);
            this->file_name = (char *)malloc(this->file_name_len);
            strcpy_s(file_name, file_name_len, file_name);
            this->file_name[file_name_len] = '\0';
            this->chunk_no = chunk_no;
            this->public_IP = public_IP;
            this->port_no = port_no;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating tracker entry failed\n";
        }
    };

    /**
     * Deconstructor for client entry in tracker
     */
    ~tracker_peer_list() {
        if(file_name != nullptr)
            free(file_name);
    };

    /**
     * Getters for attributes in entry
     * @return attribute
     */
    uint8_t get_file_name_len();

    char* get_file_name_ptr();

    uint8_t get_chunk_no();

    uint32_t get_public_IP();

    uint16_t get_port_no();

    void print_peer_list_entry();

};

class tracker_file_list {
private:
    uint8_t file_name_len;
    char *file_name;
    uint32_t no_of_chunks;
public:
    tracker_file_list(uint8_t file_name_len, char *file_name, uint32_t no_of_chunks) {
        assert(file_name_len > 0 && file_name_len < 256);
        try {
            this->file_name_len = (uint8_t) (file_name_len + 1);
            this->file_name = (char *)malloc(this->file_name_len);
            strcpy_s(file_name, file_name_len, file_name);
            this->file_name[file_name_len] = '\0';
            this->no_of_chunks = no_of_chunks;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating tracker entry failed\n";
        }
    };

    ~tracker_file_list() {
        if(file_name != nullptr)
            free(file_name);
    };

    void print_file_list_entry();

};

#endif //CS3103_P2P_TRACKER_ENTRIES_H
