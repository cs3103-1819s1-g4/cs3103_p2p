//
// Created by Jarrett on 31/10/2018.
//

#ifndef CS3103_P2P_P2P_CLIENT_HELPER_H
#define CS3103_P2P_P2P_CLIENT_HELPER_H

#include "../core/tracker_entries.h"
#include <map>

using namespace std;

int parse_peer_list(map<int, tracker_peer_list_entry>& peer_list, string response);

int choose_random_server(map<int, tracker_peer_list_entry>& peer_list,
        string& p2p_server_ip, string& p2p_server_chunk_num, string& p2p_server_port_num);

#endif //CS3103_P2P_P2P_CLIENT_HELPER_H
