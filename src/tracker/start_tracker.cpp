#include "tracker.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#pragma comment(lib, "iphlpapi.lib")
#define Q_LEN 2048  //Max length of buffer
#define PORT "80"   //The port on which to listen for incoming data

int main()
{
    tracker server(PORT);

    server.init();
    server.listen();
}