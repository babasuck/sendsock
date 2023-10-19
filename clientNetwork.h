#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

typedef struct sockaddr_in sockaddr_in;
typedef struct Client Client;

Client* createClient();
BOOL connectToServer(Client* client, const char* IP, UINT port);
SOCKET client_getSocket(Client* client);