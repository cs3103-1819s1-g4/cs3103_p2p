#ifndef CS3103_P2P_CORE_FUNCTIONS_H
#define CS3103_P2P_CORE_FUNCTIONS_H

#include <winsock2.h>
#include <ipmib.h>
#include <iphlpapi.h>
#include <assert.h>
#include <mutex>
#include <chrono>
#include <deque>
#include <cstdio>
#include <iostream>
#include <string>

extern const unsigned int Q_LEN;
extern const unsigned int PACKET_SIZE;

void get_private_IP(IN_ADDR &IP);
void print_main_server(struct sockaddr_in *addr, std::string port);

/**
 * Consumer producer queue of char buffers of packet_size for concurrent purposes
 */
class RecvBuffer {
private:
    std::mutex mu;
    std:: condition_variable cond;
    char *buffer_;
    int *data_len; //   returns the length of data of queue given the idx
    char *recv_buffer;
    int next_in;
    int next_out;
    int occupied;
    int packet_size_;
public:
    /**
     * Constructor for receving buffer, 'MAX_SIZE of packet_size' sized buffer
     * @param packet_size
     */
    explicit RecvBuffer(const unsigned int packet_size) {
        packet_size_ = packet_size;
        buffer_ = (char *)malloc(packet_size * Q_LEN);
        data_len = (int *)malloc(Q_LEN);
        recv_buffer = (char *)malloc(packet_size);
        if(buffer_ == nullptr || recv_buffer == nullptr || data_len == nullptr) {
            std::cout << "[ERROR]: " << GetLastError() << "\tFailed to allocate memory\n";
            WSACleanup();
        }
        next_in = 0;
        next_out = 0;
        occupied = 0;
    };
    // destructor
    ~RecvBuffer() {
        if(buffer_ != nullptr)
            free(buffer_);
        if(data_len != nullptr)
            free(data_len);
        if(recv_buffer != nullptr)
            free(data_len);
    };

    bool producer(SOCKET sock, sockaddr_in client_addr, int sin_size);
    void consumer(char *buffer);
};

#endif //CS3103_P2P_CORE_FUNCTIONS_H
