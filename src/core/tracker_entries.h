#ifndef CS3103_P2P_TRACKER_ENTRIES_H
#define CS3103_P2P_TRACKER_ENTRIES_H

#include <assert.h>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

class tracker_peer_list_entry {

private:

    string file_name;
    uint8_t chunk_no;
    uint8_t public_IP_len;
    string public_IP;
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
    tracker_peer_list_entry(string file_name, uint8_t chunk_no,string public_IP, uint16_t port_no) {

        try {
            this->file_name = file_name;
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
//    ~tracker_peer_list_entry() {
//        if(file_name != NULL)
//            free(file_name);
//    };

    /**
     * Getters for attributes in entry
     * @return attribute
     */

    string get_file_name_ptr();

    uint8_t get_chunk_no();

    string get_public_IP();

    uint16_t get_port_no();

    void print_peer_list_entry();

    std::string generate_message();
};

class tracker_file_list_entry {
private:
    uint8_t file_name_len;
    string file_name;
    uint32_t no_of_chunks;
public:
    tracker_file_list_entry(string file_name, uint32_t no_of_chunks) {
        assert(file_name_len > 0 && file_name_len < 256);
        try {
            this->file_name = file_name;
            this->no_of_chunks = no_of_chunks;

        } catch (std::exception &e) {
            std::cerr << "[ERROR]: " << e.what() << "\tString copy while creating tracker entry failed\n";
        }
    };

//    ~tracker_file_list_entry() {
//        if(file_name != nullptr)
//            free(file_name);
//    };

    void print_file_list_entry();
    string get_file_name();
    uint32_t get_no_of_chunk();
    void set_no_of_chunk(uint32_t num);
};

#endif //CS3103_P2P_TRACKER_ENTRIES_H
