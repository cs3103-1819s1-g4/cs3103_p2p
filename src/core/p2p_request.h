//
// Created by Jarrett on 26/10/2018.
//

#ifndef CS3103_P2P_P2P_REQUEST_H
#define CS3103_P2P_P2P_REQUEST_H

/**
 * p2p request packet (between p2p client and tracker)
 * Format: REQUEST <FLAG> [<FILENAME>] [<CHUNK NO>] [<IP ADDRESS>]
 */

#include <string>

using namespace std;

class p2p_request {
private:
    string request;
    int flag;
    string filename;
    int chunk_no;
    string public_ip;
public:
    // Constructor
    p2p_request(int flag, string filename, int chunk_no, string public_ip):
    request{"REQUEST"}, flag{flag}, filename{filename}, chunk_no{chunk_no}, public_ip{public_ip} {}

    string get_request();
    int get_flag();
    string get_filename();
    int get_chunk_no();
    string get_public_ip();

    // TODO: Do you need a destructor?
//    ~p2p_request() {
//        flag = 0;
//        chunk_no = 0;
//    }

};


#endif //CS3103_P2P_P2P_REQUEST_H
