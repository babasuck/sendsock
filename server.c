#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#define BUFFERSIZE 4096

int main() {
    WSADATA ws = { 0 };
    WSAStartup(MAKEWORD(2, 2), &ws);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(server, (struct sockaddr*)&addr, sizeof addr) == SOCKET_ERROR) {
        printf("Bind error. : %d\n" , WSAGetLastError());
        exit(0);
    }
    if(listen(server, 5) == SOCKET_ERROR){
        printf("listen error. : %d\n" , WSAGetLastError());
        exit(0);
    };
    printf("Listening..\n");
    struct sockaddr_in clientAddress = {0};
    int clientSize = sizeof(clientAddress);
    //struct timeval time = {1, 0};
    fd_set clients;
    FD_ZERO(&clients);
    FD_SET(server, &clients);
    while(1) {
        for(int i = 0; i < clients.fd_count; i++) {
            printf("Client # %d - %d\n", i, clients.fd_array[i]);
        }
        fd_set activitySockets = clients;
        int num = select(0, &activitySockets, 0, 0, 0);
        for(int i = 0; i < num; i++) {
            SOCKET curSocket = activitySockets.fd_array[i];
            if(curSocket == server) { // new client
                printf("It's server.\n");
                SOCKET newClient = accept(server, (struct sockaddr*)&clientAddress, &clientSize);
                if (newClient != INVALID_SOCKET) {
                    FD_SET(newClient, &clients);
                    printf("new client - %s\n", inet_ntoa(clientAddress.sin_addr));
                    const wchar_t* wideStr = L"Привет мир!"; // ваша строка Unicode.
                    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
                    char* utf8Str = (char*)malloc(utf8Size);
                    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, utf8Size, NULL, NULL);
                    send(newClient, utf8Str, utf8Size, 0);
                    free(utf8Str);
                }
            }
            else { // new message
                char buffer[BUFFERSIZE];
                ZeroMemory(buffer, 4096);
                int bytesRecv = 0;
                if((bytesRecv = recv(curSocket, buffer, BUFFERSIZE, 0)) <= 0) {
                    printf("client - %s close connection.\n", inet_ntoa(clientAddress.sin_addr));
                    closesocket(curSocket);
                    FD_CLR(curSocket, &clients);
                } else { // broadcast
                    printf("bytesRecv - %d mes - %s\n", bytesRecv, buffer);
                    for(int j = 0; j < clients.fd_count; j++) {
                        if(clients.fd_array[j] != server) {
                            send(clients.fd_array[j], buffer, bytesRecv, 0);
                        }
                    }
                }
            }
        }
    }
}