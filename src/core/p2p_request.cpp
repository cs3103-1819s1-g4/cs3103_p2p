//
// Created by Jarrett on 26/10/2018.
//

#include "p2p_request.h"

string p2p_request::get_request() {
    return this->request;
}

int p2p_request::get_flag() {
    return this->flag;
}

string p2p_request::get_filename() {
    return this->filename;
}

int p2p_request::get_chunk_no() {
    return this->chunk_no;
}

string p2p_request::get_public_ip() {
    return this->public_ip;
}
