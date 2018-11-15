
#include "tracker_entries.h"

string tracker_peer_list_entry::get_file_name() {
    return this->file_name;
}

uint16_t tracker_peer_list_entry::get_chunk_no() {
    return this->chunk_no;
}

string tracker_peer_list_entry::get_public_IP() {
    return this->public_IP;
}

uint16_t tracker_peer_list_entry::get_port_no() {
    return this->port_no;
}

void tracker_peer_list_entry::print_peer_list_entry() {
    std::cout << file_name << "\t" << std::to_string(chunk_no) << "\t" << public_IP << "\t" << port_no << "\n";
}

void tracker_file_list_entry::print_file_list_entry() {
    std::cout << file_name << "\t" << file_name_len;
}

string tracker_file_list_entry::get_file_name() {
    return this->file_name;
}

//uint32_t tracker_file_list_entry::get_no_of_chunk() {
//    return this->no_of_chunks;
//}
//
//void tracker_file_list_entry::set_no_of_chunk(uint32_t num) {
//    this->no_of_chunks = num;
//}
std::string tracker_peer_list_entry::generate_message() {
    std::string message = std::string(file_name) + " " + std::to_string(chunk_no) + " " +
            std::string(public_IP) + " " +  std::to_string(port_no) + "|";
    return message;
}
