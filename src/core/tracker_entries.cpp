
#include "tracker_entries.h"

uint8_t tracker_peer_list::get_file_name_len() {
    return this->file_name_len;
}

char *tracker_peer_list::get_file_name_ptr() {
    return this->file_name;
}

uint8_t tracker_peer_list::get_chunk_no() {
    return this->chunk_no;
}

char *tracker_peer_list::get_public_IP() {
    return this->public_IP;
}

uint16_t tracker_peer_list::get_port_no() {
    return this->port_no;
}

void tracker_peer_list::print_peer_list_entry() {
    std::cout << file_name << "\t" << chunk_no << "\t" << public_IP << "\t" << port_no << "\n";
}

void tracker_file_list::print_file_list_entry() {
    std::cout << file_name << "\t" << file_name_len;
}

std::string tracker_peer_list::generate_message() {
    std::string message = std::to_string(file_name_len) + std::string(file_name) + std::to_string(chunk_no) +
            std::to_string(public_IP_len) +  std::string(public_IP) +  std::to_string(port_no);
    return message;
}