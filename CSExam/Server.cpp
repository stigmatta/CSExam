#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

struct Position {
    string name, category;
    float price;
};

#define MAXSTRLEN 256

string filename = "menu.txt";
Position* menu;
int timeWaiting = 0;
int products = 0;
vector<string> history;

WSADATA wsaData;
SOCKET listenSocket, clientSocket;
sockaddr_in addr;

string generatingTimeResponse(string request) {
    for (int i = 0; i < products; i++) {
        if (strstr(request.c_str(), menu[i].name.c_str())) {
            if (menu[i].category == "Burgers")
                timeWaiting += 5;
            else if (menu[i].category == "Sides")
                timeWaiting += 4;
            else if (menu[i].category == "Desserts") {
                timeWaiting += 3;
            }
            else if (menu[i].category == "Drinks")
                timeWaiting += 2;
        }
    }
    return timeWaiting != 0 ? "Please wait for " + to_string(timeWaiting) + " seconds" : "We haven't got these positions in our menu";
}

string generatingAllResponse(string request) {
    string allResponse;
    for (int i = 0; i < products; i++) {
        if (strstr(request.c_str(), menu[i].name.c_str())) {
            ostringstream priceStream;
            priceStream << fixed << setprecision(2) << menu[i].price;
            allResponse += menu[i].name + "-" + priceStream.str() + "\n";
        }
    }
    return allResponse;
}

void endTheProgramm(SOCKET clientSocket) {
    cout << "Message history:\n";
    for (const auto& order : history) {
        cout << order << endl;
    }

    closesocket(clientSocket);
}

DWORD WINAPI listenForEndCommand(LPVOID lpParam) {
    SOCKET listenSocket = (SOCKET)lpParam;
    string endCommand;
    cin >> endCommand;
    if (endCommand == "end") {
        for (SOCKET clientSocket = 0; clientSocket <= listenSocket; ++clientSocket) {
            endTheProgramm(clientSocket);
        }
        exit(0);
    }
    return 0;
}

int main() {
    ifstream file(filename);
    string jsonString((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    size_t pos = 0;
    while ((pos = jsonString.find("name", pos)) != string::npos) {
        products++;
        pos += 4;
    }

    menu = new Position[products];
    pos = 0;
    int i = 0;

    while ((pos = jsonString.find("name", pos)) != string::npos) {
        size_t nameEnd = jsonString.find(",", pos + 4);
        menu[i].name = jsonString.substr(pos + 8, nameEnd - pos - 9);

        size_t categoryStart = jsonString.find("category", nameEnd);
        size_t categoryEnd = jsonString.find(",", categoryStart + 12);
        menu[i].category = jsonString.substr(categoryStart + 12, categoryEnd - categoryStart - 13);

        size_t priceStart = jsonString.find("price", categoryEnd);
        size_t priceEnd = jsonString.find(",", priceStart + 8);
        menu[i++].price = stof(jsonString.substr(priceStart + 8, priceEnd - priceStart - 8));
        pos = priceEnd;
    }

   

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    addr.sin_port = htons(20000);

    bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr));
    listen(listenSocket, SOMAXCONN);

    fd_set readfds;
    int maxfd, res;

    FD_ZERO(&readfds);
    FD_SET(listenSocket, &readfds);
    maxfd = listenSocket;

    CreateThread(0, NULL, listenForEndCommand, NULL, 0, 0);

    while (true) {
        fd_set tmpfds = readfds;
        res = select(maxfd + 1, &tmpfds, NULL, NULL, NULL);
        if (res < 0) {
            cout << "Error in select\n";
            break;
        }
        else if (res == 0) {
            cout << "Timeout\n";
            continue;
        }

        for (SOCKET clientSocket = 0; clientSocket <= maxfd; ++clientSocket) {
            if (FD_ISSET(clientSocket, &tmpfds)) {
                if (clientSocket == listenSocket) {
                    clientSocket = accept(listenSocket, NULL, NULL);
                    FD_SET(clientSocket, &readfds);
                    maxfd = max(maxfd, clientSocket);
                    cout << "New client connected" << endl;
                }
                else {
                    timeWaiting = 0;
                    char buf[MAXSTRLEN];
                    res = recv(clientSocket, buf, MAXSTRLEN, 0);
                    if (res <= 0) {
                        cout << "Client disconnected" << endl;
                        closesocket(clientSocket);
                        FD_CLR(clientSocket, &readfds);
                    }
                    else {
                        buf[res] = '\0';
                        string request(buf);
                        if(request!="Client is waiting")
                            history.push_back(request);

                        string timeResponse = generatingTimeResponse(request);
                        send(clientSocket, timeResponse.c_str(), timeResponse.length(), 0);
                        Sleep(timeWaiting * 1000);
                        string allResponse = generatingAllResponse(request);
                        send(clientSocket, allResponse.c_str(), allResponse.length(), 0);
                    }
                }
            }
        }

    }
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}
