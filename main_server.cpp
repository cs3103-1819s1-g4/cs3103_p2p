
#include "main_server.h"

#define MAX_CONNECTIONS 10

Main_Server::Main_Server() : sock(INVALID_SOCKET), online(false) {

    WSADATA wsock;
    int status = WSAStartup(MAKEWORD(2,2),&wsock);

    if ( status != 0)
        std::cout << "[ERROR]: " << status << " Unable to start Winsock." << std::endl;
    else
        online = true;

    ZeroMemory(&server_IP, sizeof(server_IP));
    get_local_IP(server_IP);
}

bool Main_Server::start(const char *port) {

    stop();

    struct addrinfo *result = nullptr, hints;
    int status;

    ZeroMemory(&result, sizeof (result));
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_flags = AI_PASSIVE; // to allow binding
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_UDP;

    status = getaddrinfo(inet_ntoa(server_IP), port, &hints, &result);
    if (status != 0) {
        std::cout << "[ERROR]: " << status << " Unable to get address info for Port " << port << "." << std::endl;
        return false;
    }

    SOCKET serv_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_sock == INVALID_SOCKET) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to create Socket." << std::endl;
        freeaddrinfo(result);
        return false;
    }

    int on = 1;
    if( (setsockopt(serv_sock, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on))) < 0 ) {
        perror("setsockopt");
        return false;
    }

    if (bind(serv_sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cout << "[ERROR]: " << WSAGetLastError() << " Unable to bind Socket." << std::endl;
        freeaddrinfo(result);
        closesocket(serv_sock);
        return false;
    }

    // this is temporary, should be transferred to core_functions.cpp
    struct sockaddr_in *addr;
    addr = (struct sockaddr_in *)result->ai_addr;
    std::cout << "P2P server running at IP address: " << inet_ntoa((struct in_addr)addr->sin_addr) << "\tport number: " << port << std::endl;

    // We don't need this info any more
    freeaddrinfo(result);
    sock = serv_sock;
    return true;
}

Main_Server::~Main_Server() {

    stop();
    if (online)
        WSACleanup();
}

void Main_Server::stop() {

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}

void Main_Server::get_local_IP(IN_ADDR &IP) {

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
                          NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
                          (LPTSTR) &lpMsgBuf, 0, NULL)) {
            printf("\tError: %s", lpMsgBuf);
            LocalFree(lpMsgBuf);
        }
        exit(1);
    }

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
        localhost_IP_addr_table = nullptr;
    }
}
