#include <iostream>
#include <string>
#include "WinSock2.h" 
#include <ws2tcpip.h> 
#pragma comment(lib, "Ws2_32.lib") 

using namespace std;

WSADATA wsaData;
SOCKET _socket;
sockaddr_in addr;
#define MAXSTRLEN 256

int main() {
    system("title Client");

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;

    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    addr.sin_port = htons(20000);
    connect(_socket, (SOCKADDR*)&addr, sizeof(addr));

    string order;
    char resp[MAXSTRLEN];

    while (true) {
        getline(cin, order);

        if (order == "exit")
            break;

        send(_socket, order.c_str(), order.length(), 0);

        int i = recv(_socket, resp, MAXSTRLEN - 1, 0);
        if (i > 0) {
            resp[i] = '\0';
            cout << resp << endl;
        }
        else {
            cout << "No response from server." << endl;
        }


        i = recv(_socket, resp, MAXSTRLEN - 1, 0);
        if (i > 0) {
            resp[i] = '\0';
            cout << resp << endl;
        }
        else {
            cout << "No response from server." << endl;
        }
        
    }

    closesocket(_socket);
    WSACleanup();

    return 0;
}
