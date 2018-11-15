//
// Created by Jarrett on 31/10/2018.
//

#include <vector>
#include <sstream>
#include "p2p_client_helper.h"

using namespace std;

int index = 0;

/**
 * Parses the response from tracker and saves the content as peer list map
 * @param peer_list
 * @param response
 * @return size of the peer list
 */

int parse_peer_list(map<int, tracker_peer_list_entry>& peer_list, string response) {

    string space_delimiter = " ";
    string entry_delimiter = "|";
    vector<string> entry_tokens;
    size_t pos = 0;

    // Remove "Response "
    response.erase(0, response.find(space_delimiter) + space_delimiter.length());

    // Loops through the peer list entry
    while ((pos = response.find(entry_delimiter)) != string::npos) {

        string entry = response.substr(0, pos);

        // For each peer list entry, parse it into a tracker_peer_list_entry object
        stringstream entry_stream(entry);
        string entry_token;
        while(getline(entry_stream, entry_token, ' ')) {
            entry_tokens.push_back(entry_token);
        }
        string filename = entry_tokens[0];
        uint16_t chunk_num = (uint16_t) stoi(entry_tokens[1]);
        string public_ip = entry_tokens[2];
        uint16_t port_num = (uint16_t) stoi(entry_tokens[3]);

        cout << filename << " " << chunk_num << " " << public_ip << " " << port_num << endl;

        tracker_peer_list_entry entry_obj(filename, chunk_num, public_ip, port_num);
        peer_list.insert(pair<int, tracker_peer_list_entry> (index, entry_obj));
        index++;

        response.erase(0, response.find(entry_delimiter) + entry_delimiter.length());
        entry_tokens.clear();
    }


    return peer_list.size();
}

int choose_random_server(map<int, tracker_peer_list_entry>& peer_list,
                         string& p2p_server_ip, string& p2p_server_chunk_num, string& p2p_server_port_num) {

    int min = 0;
    int max = peer_list.size() - 1;
    map<int, tracker_peer_list_entry>::iterator it;
    int rand_num = rand()%(max-min + 1) + min; // TODO: As for now, use rand()

    it = peer_list.begin();
    while(rand_num > 0) {
        it++;
        rand_num--;
    }
    tracker_peer_list_entry entry_obj = it->second;

    p2p_server_ip = entry_obj.get_public_IP();
    p2p_server_chunk_num = to_string(entry_obj.get_chunk_no());
    p2p_server_port_num = to_string(entry_obj.get_port_no());

    peer_list.erase(it);


    return 0;
}
