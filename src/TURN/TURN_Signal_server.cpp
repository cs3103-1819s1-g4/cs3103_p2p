

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <map>
#include <iterator>
#include <thread>
#include <iostream>
#include <future>

void setupUDPserver(int param)
{

    int sockUDP;
    int bytes_readUDP;
    unsigned int addr_lenUDP;
    char recv_dataUDP[1024], reply_dataUDP[1024];
    struct sockaddr_in server_addrUDP, client_addrUDP;

    //Create a datagram socket (connection less)
    if ((sockUDP = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addrUDP.sin_family = AF_INET;
    server_addrUDP.sin_port = htons(6883);
    server_addrUDP.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addrUDP.sin_zero), 8);

    //bind the sockt to local Internet Address (IP and PORT).
    if (bind(sockUDP, (struct sockaddr *)&server_addrUDP,
             sizeof(struct sockaddr)) == -1)
    {
        perror("Bind");
        exit(1);
    }

    addr_lenUDP = sizeof(struct sockaddr);

    printf("\n UDPServer Waiting for DATA from client on port 6883");
    fflush(stdout);

    while (1)
    {
        //receive data from the client
        bytes_readUDP = recvfrom(sockUDP, recv_dataUDP, 1024, 0,
                                 (struct sockaddr *)&client_addrUDP, &addr_lenUDP);
        //recv_dataUDP[bytes_readUDP] = '\0';

        if(bytes_readUDP == 0 || bytes_readUDP == -1 || recv_dataUDP[0] == ' '){
            continue;
        }


        char tempRecv_dataUDP[bytes_readUDP];
        memcpy(tempRecv_dataUDP, recv_dataUDP, bytes_readUDP);
        tempRecv_dataUDP[64] = '\0';

        //process data. Here, we just print it and Reply to Client
        printf("\n(%s , %d) said : ", inet_ntoa(client_addrUDP.sin_addr),
               ntohs(client_addrUDP.sin_port));
        printf("%s", tempRecv_dataUDP);
        fflush(stdout);

        std::string str(tempRecv_dataUDP);
        // char *type = strtok(tempRecv_dataUDP, " ");
        if (strncmp(tempRecv_dataUDP, "getPublic",9)== 0)
        {
            // get sendback public ip:port
            strcpy(reply_dataUDP, "yourIP");
            char ipAddUDP[50];
            char portNumUDP[50];
            sprintf(ipAddUDP, "%s", inet_ntoa(client_addrUDP.sin_addr));
            sprintf(portNumUDP, "%d", ntohs(client_addrUDP.sin_port));
            strcat(reply_dataUDP, ipAddUDP);
            strcat(reply_dataUDP, ":");
            strcat(reply_dataUDP, portNumUDP);
            sendto(sockUDP, reply_dataUDP, strlen(reply_dataUDP), 0,
                   (struct sockaddr *)&client_addrUDP, addr_lenUDP);
        }
        else if (str.find(':') == std::string::npos || str.find(':') == 0 || ':' == str.back())
        {
            //printf("test\n\n");
        }
        else
        {
            // relay signal to socket
                        //printf("test4\n\n");

            char tempRecv_data[bytes_readUDP];
            memcpy(tempRecv_data, recv_dataUDP, bytes_readUDP);
            tempRecv_data[bytes_readUDP] = '\0';
            char *typeOrIp = strtok(tempRecv_data, " ");
            //printf("test1\n\n");

            char dest[1024];
            strcpy(dest, typeOrIp);
            std::string str(dest);
            if (str.find(':') == std::string::npos || str.find(':') == 0 || ':' == str.back()) continue;
            char *token = strtok(dest, ":");
            char destIP[1024];
            char destPort[1024];
            strcpy(destIP, token);
            token = strtok(NULL, ":");
            strcpy(destPort, token);

            //printf("\n%s??:%s\n\n\n",destIP,destPort);
            int relayToSock;
            char send_data[1024];
            struct hostent *host;
            struct sockaddr_in server_addr;
            host = gethostbyname(destIP);
            server_addr.sin_family = AF_INET;
            int j;
            bool isbroken = false;
            for (j=0; destPort[j]; j++) {
                if (!isdigit(destPort[j])) {
                    //printf("Bad\n");
                    isbroken = true;
                    break;
                }
            }   
            std::cout<< isbroken;
            if(isbroken) continue;
            server_addr.sin_port = htons(atoi(destPort));
            server_addr.sin_addr = *((struct in_addr *)host->h_addr);
            bzero(&(server_addr.sin_zero), 8);
            int i = 0;
            while (recv_dataUDP[i] != ' ')
            {
                i++;
            }
            i++;
            char *data = &recv_dataUDP[i];
            sendto(sockUDP, data, bytes_readUDP - i, 0,
                   (struct sockaddr *)&server_addr, addr_lenUDP);
        }
    }
}

std::mutex threadMutex;
std::map<std::string, std::thread> connectionThreads;
std::map<std::string, int> connectionSocketFds;

void removeThread(std::thread::id id)
{
    //std::lock_guard<std::mutex> lock(threadMutex);

    std::map<std::string, std::thread>::iterator itr;
    // std::cout << "\nThe map sockets is : \n";
    // std::cout << "\tKEY\tELEMENT\n";
    threadMutex.lock();
    for (itr = connectionThreads.begin(); itr != connectionThreads.end(); ++itr)
    {
        if (itr->second.get_id() == id)
        {
            itr->second.detach();
            connectionSocketFds.erase(itr->first);
            connectionThreads.erase(itr);
            break;
        }
        //std::cout  <<  '\t' << itr->first << '\n';
    }
    std::map<std::string, int>::iterator itr2;
    for (itr2 = connectionSocketFds.begin(); itr2 != connectionSocketFds.end(); ++itr2)
    {
        std::cout << '\t' << itr2->first << "\t" << itr2->second << '\n';
    }
    threadMutex.unlock();
    //std::cout << std::endl;
}

int getSockFromIP(std::string ipPort)
{
    std::map<std::string, int>::iterator itr;
    threadMutex.lock();
    std::cout << ipPort << std::endl
              << std::endl;
    itr = connectionSocketFds.find(ipPort);
    threadMutex.unlock();
    if (itr == connectionSocketFds.end())
    {
        return -1;
    }
    return itr->second;
}

void relayTCPconnection(int fromSockFd, int toSockFd)
{
    char buffer[65556];
    //std::cout<< "start relaying" << std::endl;
    int bytesRead = 0;
    int result;
    int x = 65556;
    while (bytesRead < x)
    {
        result = recv(fromSockFd, buffer + bytesRead, x - bytesRead,0);
        if (result < 1 )
        {
            // Throw your error.
            break;
        }

        bytesRead += result;
    }
    send(toSockFd, buffer, bytesRead, 0);
    close(toSockFd);
    std::cout << "done relaying" << std::endl;
}

void setupConnectionForTCP(int connectedSocketFd, sockaddr_in client_addr)
{
    char reply[1024], recv_data[1024];
    int bytes_recieved;

    while (1)
    {
        //receive data from the client
        bytes_recieved = recv(connectedSocketFd, recv_data, 1024, 0);
        if (bytes_recieved == 0 || bytes_recieved == -1)
        {
            close(connectedSocketFd); //close connect socket used for the client
            printf("\n Connection is closed with (%s , %d)",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            std::async(removeThread, std::this_thread::get_id());
            return;
        }
        if(recv_data[0] == ' '){
            char send_data[1024];
            strcpy(send_data, "404 Not found Please Send ip:port again");
            send(connectedSocketFd, send_data, strlen(send_data), 0);
            continue;
        }

        //   recv_data[bytes_recieved] = '\0';
        printf("\nRecieved A first tcp message from (%s , %d)",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        // printf("\n RECIEVED  DATA = %s ", recv_data);
        fflush(stdout);

        char tempRecv_data[bytes_recieved];
        memcpy(tempRecv_data, recv_data, bytes_recieved);
        tempRecv_data[bytes_recieved] = '\0';
        std::string str(tempRecv_data);

        if (strncmp(tempRecv_data, "getPublic",9) == 0)
        {
            // get sendback public ip:port
            strcpy(reply, "");
            char ipAdd[50];
            char portNum[50];
            sprintf(ipAdd, "%s", inet_ntoa(client_addr.sin_addr));
            sprintf(portNum, "%d", ntohs(client_addr.sin_port));
            strcat(reply, ipAdd);
            strcat(reply, ":");
            strcat(reply, portNum);
            send(connectedSocketFd, reply, strlen(reply), 0);
        }
        else if (str.find(':') == std::string::npos || str.find(':') == 0 || ':' == str.back())
        {
            char send_data[1024];
            strcpy(send_data, "404 Not found Please Send ip:port again1");
            send(connectedSocketFd, send_data, strlen(send_data), 0);
        }
        else
        {
            char dest[1024];
            // char *typeOrIp = strtok(tempRecv_data, " ");
            strcpy(dest, tempRecv_data);
            std::string str(dest);
            if (str.find(':') == std::string::npos || str.find(':') == 0 || ':' == str.back()) {
                char send_data[1024];
                strcpy(send_data, "404 Not found Please Send ip:port again2");
                send(connectedSocketFd, send_data, strlen(send_data), 0);
                continue;
            }
            char *token = strtok(dest, ":");
            char destIP[1024];
            char destPort[1024];
            strcpy(destIP, token);
            token = strtok(NULL, ":");
            strcpy(destPort, token);
            std::string desStrIP(destIP);
            std::string desStrPort(destPort);
            //std::cout<< getSockFromIP(typeOrIp) << std::endl;
            std::string ipport = "";
            ipport.append(desStrIP);
            ipport.append(":");
            ipport.append(desStrPort);
            int toSendSockFd = getSockFromIP(ipport);
            printf("%d\n\n", toSendSockFd);
            if (toSendSockFd == -1 || toSendSockFd == connectedSocketFd)
            {
                char send_data[1024];
                strcpy(send_data, "404 Not found Please Send ip:port again3");
                send(connectedSocketFd, send_data, strlen(send_data), 0);
            }
            else
            {
                char send_data[1024];
                strcpy(send_data, "200 OK start sending data");
                send(connectedSocketFd, send_data, strlen(send_data), 0);
                relayTCPconnection(connectedSocketFd, toSendSockFd);
            }
        }
    }

    // close(connectedSocketFd); //close connect socket used for the client
    // printf("\n Connection is closed with (%s , %d)",
    //        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    // std::async(removeThread, std::this_thread::get_id());
}

void waitForConnection(unsigned int sin_size, int connected, int sock, sockaddr_in client_addr)
{
    //std::lock_guard<std::mutex> lock(threadMutex);
    sin_size = sizeof(struct sockaddr_in);
    //Accept connection request from the client and create new socket (name:connected) for the client
    //"Connect socket"
    connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
    printf("\n I got a connection from (%s , %d)\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    std::string ipPort = inet_ntoa(client_addr.sin_addr);
    ipPort.append(":");
    ipPort.append(std::to_string(ntohs(client_addr.sin_port)));
    //std::cout << ipPort << std::endl;
    threadMutex.lock();
    connectionThreads.insert(std::pair<std::string, std::thread>(ipPort, std::thread(setupConnectionForTCP, connected, client_addr)));
    connectionSocketFds.insert(std::pair<std::string, int>(ipPort, connected));

    std::map<std::string, int>::iterator itr2;
    for (itr2 = connectionSocketFds.begin(); itr2 != connectionSocketFds.end(); ++itr2)
    {
        std::cout << '\t' << itr2->first << "\t" << itr2->second << '\n';
    }

    threadMutex.unlock();
}

void setupTCPserver(int param)
{
    int sock, connected, isTrue = 1;

    struct sockaddr_in server_addr, client_addr;
    unsigned int sin_size;

    //Create a "Listening socket" or "Server socket"
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &isTrue, sizeof(int)) == -1)
    {
        perror("Setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6882);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    //bind the socket to local Internet Address (IP and PORT).

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Unable to bind");
        exit(1);
    }

    //Listening for connections from client.
    //Maximum 5 connection requests are accepted.
    if (listen(sock, 5) == -1)
    {
        perror("Listen");
        exit(1);
    }

    printf("\nMy TCP ECHO Server is Waiting for client on port 6882");
    fflush(stdout);

    // parent
    while (1)
    {
        waitForConnection(sin_size, connected, sock, client_addr);
    }
    close(sock);
}

int main() // TCP Concurrent Server
{

    int processID; //for concurrent server

    //processID = fork();
    int param = 1;
    //***********UDP Listener**************
    std::thread udpServerThread(setupUDPserver, param);

    //***********TCP Listener**************
    std::thread tcpServerThread(setupTCPserver, param);

    std::map<int, int> gquiz1; // empty map container

    udpServerThread.join();
    tcpServerThread.join();
    return 0;
}