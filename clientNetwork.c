#include "clientNetwork.h"

struct Client {
    SOCKET clientSocket;
    sockaddr_in clientAddress;
};

struct Client* createClient() {
    struct Client* client = (struct Client*)malloc(sizeof(*client));
    WSADATA wsData = {0};
    sockaddr_in* clientAddress = (sockaddr_in*)malloc(sizeof(*clientAddress));
    WSAStartup(MAKEWORD(2, 2), &wsData);
    SOCKET user = socket(AF_INET, SOCK_STREAM, 0);
    client->clientSocket = user;
    return client;
}

int connectToServer(struct Client* client, const char* IP, UINT port) {
    sockaddr_in serverAdress = {0};
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(port);
    serverAdress.sin_addr.s_addr = inet_addr(IP);
    return connect(client->clientSocket, (struct sockaddr*)&serverAdress, sizeof serverAdress);
}

SOCKET client_getSocket(struct Client* client) {
    return client->clientSocket;
}