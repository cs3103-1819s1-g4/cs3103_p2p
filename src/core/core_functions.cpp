#include "core_functions.h"
/**
 * Producer function of consumer producer queue of buffers.
 * Each time a datagram is received. It is stored in the queue at idx 'next_in'.
 * @param sock
 * @param client_addr
 * @param sin_size
 */
bool RecvBuffer::producer(SOCKET sock, sockaddr_in client_addr, int sin_size) {

    std::unique_lock<std::mutex> locker(mu); // lock mutex

    // wait for size of queue < max size before proceeding
    cond.wait(locker, [this](){return occupied < Q_LEN;});
    assert(occupied < Q_LEN);

    int bytes_recv;
    std::cout << "Waiting to receive datagram...\n";
    if ((bytes_recv = recvfrom(sock, recv_buffer, packet_size_, 0, (struct sockaddr *)&client_addr,
                               &sin_size)) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << "\tReceive at socket failed\n";
        WSACleanup();
        return false;
    }
    std::cout << "Received a datagram packet from:" << inet_ntoa(client_addr.sin_addr) << "\tport no: "
    << ntohs(client_addr.sin_port) << "\n";

    recv_buffer[bytes_recv] = '\0';

    data_len[next_in] = bytes_recv + 1; // store the length of data
    memcpy((void *)(buffer_ + next_in * packet_size_), recv_buffer, (size_t)(data_len[next_in])); //copy over to main buffer
    occupied++;                         // increment num of packets in queue by 1
    next_in = (next_in + 1) % Q_LEN;    // idx of next available slot
    locker.unlock();    // unlock mutex
    cond.notify_all();  // unblock threads that have been blocked on a conditional variable

    return true;
}

/**
 * Consumer function of consumer producer queue of buffers.
 * Each time a datagram is loaded into the queue by the producer, processor threads can
 * use this function to load the datagram to their own buffer specified at param.
 * @param b
 */
void RecvBuffer::consumer(char *b) {

    std::unique_lock<std::mutex> locker(mu); // lock mutex

    // wait for size of queue > 0 before proceeding
    cond.wait(locker, [this](){return occupied > 0;});
    assert(occupied > 0);

    memcpy(b, (void *)(buffer_ + next_in * packet_size_), (size_t)(data_len[next_out]) ); //copy over to buffer b
    occupied--;
    next_out = (next_out + 1) % Q_LEN;
    locker.unlock();    // unlock mutex
    cond.notify_all();  // unblock threads that have been blocked on a conditional variable
}

/**
 * This functions returns the one of the private IP address assigned by local DHCP.
 * Can be edited find all the private IPs of the machine.
 * @param IP
 */
void get_private_IP(IN_ADDR &IP) {

    /* Variables used by GetIpAddrTable */
    PMIB_IPADDRTABLE localhost_IP_addr_table;
    DWORD dwRetVal = 0, dwSize = 0;//32 bits data type

    /* Variables used to return error message */
    LPVOID lpMsgBuf;

    localhost_IP_addr_table = (MIB_IPADDRTABLE *) malloc(sizeof(MIB_IPADDRTABLE));

    if (localhost_IP_addr_table) {
        // Make an initial call to GetIpAddrTable to get the
        // necessary size into the dwSize variable
        if (GetIpAddrTable(localhost_IP_addr_table, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
            free(localhost_IP_addr_table);
            localhost_IP_addr_table = (MIB_IPADDRTABLE *) malloc(dwSize);

        }
        if (localhost_IP_addr_table == nullptr) {
            perror("Memory allocation failed for GetIpAddrTable");
            exit(1);
        }
    }

    if ((dwRetVal = GetIpAddrTable(localhost_IP_addr_table, &dwSize, 0)) != NO_ERROR) {
        printf("GetIpAddrTable failed with error %d\n", dwRetVal);
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                          nullptr, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
                          (LPTSTR) &lpMsgBuf, 0, nullptr)) {
            std::cout << "\tError: " << lpMsgBuf << "\n";
            LocalFree(lpMsgBuf);
        }
        exit(1);
    }

    // We can loop through all IP addresses assigned to the local computer here
    for (int i = 0; i < (int) localhost_IP_addr_table->dwNumEntries; i++) {
        IP.S_un.S_addr = (u_long) localhost_IP_addr_table->table[i].dwAddr;
        // if IP is not the loopback address
        if (strcmp(inet_ntoa(IP), "127.0.0.1") != 0) {
            break;
        }
    }

    // table not needed anymore
    if(localhost_IP_addr_table) {
        free(localhost_IP_addr_table);
    }
}

/**
 * Print the IP address and port number the main server is running.
 * @param addr
 */
void print_main_server(struct sockaddr_in *addr, std::string port) {
    std::cout << "P2P server running at IP address: " << inet_ntoa((struct in_addr)addr->sin_addr)
            << "\tport number: " << port << "\n";
}
