//
// Created by Core on 22/10/2018.
//
#include <algorithm>
#include "tracker.h"
#include "../core/tracker_entries.h"


SOCKET listen_sock;
struct sockaddr_in si_other;
int slen = sizeof(si_other) , recv_len;
char buf[PACK_SIZE];
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
BOOL bNewBehavior = FALSE;
DWORD dwBytesReturned = 0;



void tracker::init() {

    struct addrinfo *result = nullptr, hints{};
    int status;

    ZeroMemory(&result, sizeof (result));
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;


    status = getaddrinfo(inet_ntoa(server_ip), port, &hints, &result);
    if (status != 0) {
        std::cout << "[ERROR]: " << status << " Unable to get address info for Port " << port << ".\n";
    }

    SOCKET serv_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_sock == INVALID_SOCKET) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to create Socket.\n";
        freeaddrinfo(result);
    }

    if (::bind(serv_sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket.\n";
        freeaddrinfo(result);
        closesocket(serv_sock);
    }

    print_server((struct sockaddr_in *)result->ai_addr, port, "Tracker server");
    std::cout.flush();

    // We don't need this info any more
    freeaddrinfo(result);
    listen_sock = serv_sock;
    WSAIoctl(listen_sock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);

}
//keep listening for data

void tracker::listen() {
    while (true) {
        printf("Waiting for data...");
        fflush(stdout);

//clear the buffer by filling null, it might have previously received data
        memset(buf, '\0', PACK_SIZE);

//try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(listen_sock, buf, PACK_SIZE, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR) {
            printf("recvfrom() failed with error code : %d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

//print details of the client/peer and the data received
        printf("Data: %s\n", buf);
        int request = buf[8] - '0';

        std::string message(buf);
        //int pos = message.find("|") + 1;
        //printf("%s\n", message.substr(pos).c_str());

        string reply = "dummy";
        switch (request) {
            //Download a file from the swarm. FILENAME value must be filled.
            case 1:
                reply = generateList(message);
                printf("Generating List %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Ask the tracker for the updated list. FILENAME must be filled.
            case 2:
                reply = generateList(message);
                printf("Generating List %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Inform the tracker that a chunk has been successfully downloaded. FILENAME and CHUNK NO must be filled.
            case 3:
                reply = addEntry(message) ;
                //printf("Adding Entry from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Upload a new file. FILENAME, CHUNK NO and IP ADDRESS must be filled.
            case 4:
                reply = addFile(message) ;
                //printf("Adding list of entry from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Exit from swarm. IP ADDRESS must be filled.
            case 5:
                reply = deleteIP(message);
                break;
                //Query the tracker for a list of files available.
            case 6:
                reply = query();
                printf("Replying query from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;
                //Query for a specified file. FILENAME must be filled.
            case 7:
                reply = queryFile(message);
                printf("Replying query from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                break;

            case 8:
                //update IP for STUN server
                //updateIP(message);
                break;
        }


        if(reply.length() > 0) {
            strcpy(buf, reply.c_str());
            printf("%s\n", buf);
            // TODO: Stop tracker from crashing when attempting to send
            if (sendto(listen_sock, buf, strlen(buf), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
                printf("sendto() failed with error code : %d", WSAGetLastError());
            }
        }
        memset(buf, '\0', PACK_SIZE);
    }

    closesocket(listen_sock);
    WSACleanup();

}
string tracker::addEntry(string message){
    //extract IP and Port from message
    char * ptr = NULL;
    char *cstr = new char[message.length() + 1];
    strcpy(cstr,message.c_str());
    ptr = strtok(cstr," ");
    ptr = strtok(nullptr," ");
    ptr = strtok(nullptr," ");
    string public_IP(ptr);
    ptr = strtok(NULL," ");
    string public_port(ptr);

    ptr = strtok(NULL,"|");
    while(ptr != NULL) {
        string temp(ptr);
        peer_list.emplace_back(new tracker_peer_list_entry(temp,public_IP,stoi(public_port)));
        cout << "Added " << peer_list.back()->generate_message()<<"\n";
        ptr = strtok(NULL,"|");
    }
    bool exist = false;
    for (auto &i: file_list) {
        if (peer_list.back()->get_file_name() == i->get_file_name()) {
            exist = true;
            break;
        }
    }
    if (!exist) {
        file_list.emplace_back(
                new tracker_file_list_entry(peer_list.back()->get_file_name()));
    }
    return "";
}
string tracker::addFile(string message){
    //extract IP and Port from message
    char * ptr = NULL;
    char *cstr = new char[message.length() + 1];
    strcpy(cstr,message.c_str());
    ptr = strtok(cstr," ");
    ptr = strtok(nullptr," ");
    ptr = strtok(nullptr," ");
    string public_IP(ptr);
    ptr = strtok(NULL," ");
    string public_port(ptr);

    ptr = strtok(NULL,"|");
    while(ptr != NULL) {
        string temp(ptr);
        peer_list.emplace_back(new tracker_peer_list_entry(temp,public_IP,stoi(public_port)));
        cout << "Added " << peer_list.back()->generate_message()<<"\n";
        ptr = strtok(NULL,"|");
    }
    bool exist = false;
    for (auto &i: file_list) {
        if (peer_list.back()->get_file_name() == i->get_file_name()) {
            exist = true;
            break;
        }
    }
    if (!exist) {
        file_list.emplace_back(
                new tracker_file_list_entry(peer_list.back()->get_file_name()));
    }
    return "";
}
string tracker::query(){
    string result = "RESPONSE ";
    if(file_list.empty())
    {
        return result;
    }
    for(auto &i: file_list) {
        result += i->get_file_name() + ", ";
    }
    return result.substr(0,result.length()-2);
}
string tracker::queryFile(string message){

    string fileName = message.substr(10,message.length());
    string result = "RESPONSE ";
    for(auto &i: file_list) {
        if(fileName == i->get_file_name()) {
            result += i->get_file_name();
            return result;
        }
    }
    result += "Not Found";
    return result;
}
string tracker::generateList(string message){
    string result = "RESPONSE ";
//    string filename = message.substr(10,message.find("|",10)); // TODO: Please delete this line

    // The following removes "REQUEST 1 ", thereby leaving only the filename. message == filename.
    string space_delimiter = " ";
    message.erase(0, message.find(space_delimiter) + space_delimiter.length());
    message.erase(0, message.find(space_delimiter) + space_delimiter.length());

    for(auto &i: peer_list) {
        if (i->get_file_name() == message) {
            result += i->generate_message();
        }
    }
    return result;
}
string tracker::updateIP(string message){
    return 0;
}
string tracker::deleteIP(string message){
    //string filename = message.substr(10,message.find("|",10));
//    for(vector<tracker_peer_list_entry*>::iterator iter = peer_list.begin(); iter != peer_list.end(); "blank") {
//        if (((*iter)->get_public_IP() == inet_ntoa(si_other.sin_addr))) {
//            peer_list.erase(iter);
//        } else {
//            ++iter;
//        }
//    }
    char * ptr = NULL;
    char *cstr = new char[message.length() + 1];
    strcpy(cstr,message.c_str());
    ptr = strtok(cstr," ");
    ptr = strtok(nullptr," ");
    ptr = strtok(nullptr," ");
    string public_IP(ptr);
    printf("Deleting Entries from %s\n",public_IP.c_str());
    peer_list.erase(
            remove_if(peer_list.begin(),
                    peer_list.end(),
                    [&public_IP](tracker_peer_list_entry* const & t) {return t->get_public_IP() == public_IP;}
            ),
            peer_list.end()
            );
    file_list.clear();
    for (auto &i: peer_list) {
        bool exist = false;
        for (auto &j: file_list) {
            if (i->get_file_name() == j->get_file_name()) {
                exist = true;
                break;
            }
        }
        if(!exist){
            file_list.emplace_back(
                    new tracker_file_list_entry(i->get_file_name()));
        }
    }
    
    return "";
}
