#ifndef CS3103_P2P_TRACKER_ENTRIES_H
#define CS3103_P2P_TRACKER_ENTRIES_H

/**
 * This header file contains the 2 types of entries a tracker has - file and client
 */
#include <assert.h>
#include <cstdint>
#include <iostream>


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
    TrackerClientEntry(uint8_t file_name_len, char *file_name_buffer, uint8_t chunk_no, uint32_t public_IP
            , uint16_t port_no) {
        assert(file_name_len > 0);
        try {
            this->file_name_len = file_name_len;
            this->file_name = (char *)malloc(file_name_len);
            strcpy_s(file_name, file_name_len, file_name_buffer);
            this->chunk_no = chunk_no;
            this->public_IP = public_IP;
            this->port_no = port_no;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating tracker entry failed\n";
        }
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

class TrackerFileEntry {
private:
    uint8_t file_name_len;
    char *file_name;
public:
    TrackerFileEntry(uint8_t file_name_len, char *file_name_buffer) {
        assert(file_name_len > 0);
        try {
            this->file_name_len = file_name_len;
            this->file_name = (char *)malloc(file_name_len);
            strcpy_s(file_name, file_name_len, file_name_buffer);

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating tracker entry failed\n";
        }
    };

    ~TrackerFileEntry() {
        if(file_name != nullptr)
            free(file_name);
    };

    void print_entry() {
        std::cout << file_name << "\t" << file_name_len;
    }
};
#endif //CS3103_P2P_TRACKER_ENTRIES_H
