#ifndef CS3103_P2P_TRACKER_CLIENT_ENTRY_H
#define CS3103_P2P_TRACKER_CLIENT_ENTRY_H

#include <assert.h>
#include <cstdint>
#include <iostream>

// This needs to be declared here to be a compile time constant. Global variables are run time constants
static const unsigned int MAX_FILE_NAME_LEN = 256;

class TrackerClientEntry {
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
     * @param chunk_no
     * @param public_IP
     * @param port_no
     */
    TrackerClientEntry(uint8_t file_name_len, uint8_t chunk_no, uint32_t public_IP, uint16_t port_no) {
        assert(file_name_len > 0);
        this->file_name_len = file_name_len;
        this->file_name = (char *)malloc(file_name_len);
        this->chunk_no = chunk_no;
        this->public_IP = public_IP;
        this->port_no = port_no;
    };

    /**
     * deconstructor for client entry in tracker
     */
    ~TrackerClientEntry() {
        if(file_name != nullptr)
            free(file_name);
    };

    /**
     * Getters for attributes in entry
     * @return attribute
     */
    uint8_t get_file_name_len() {
        return this->file_name_len;
    }

    char* get_file_name_ptr() {
        return this->file_name;
    }

    uint8_t get_chunk_no() {
        return this->chunk_no;
    }

    uint32_t get_public_IP() {
        return this->public_IP;
    }

    uint16_t get_port_no() {
        return this->port_no;
    }

    /**
     * Print client entry
     */
    void print_entry() {
        std::cout << file_name << "\t" << chunk_no << "\t" << public_IP << "\t" << port_no << "\n";
    };
};
#endif //CS3103_P2P_TRACKER_CLIENT_ENTRY_H
