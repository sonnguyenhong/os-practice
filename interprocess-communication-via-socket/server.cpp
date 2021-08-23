#include<windows.h>
#include<winsock2.h>
#include<winsock.h>
#include<ws2tcpip.h>
#include<stdio.h>
#include<string.h>

#define PORT 8080

#pragma comment(lib, "Ws2_32.lib")


DWORD WINAPI serverReceive(LPVOID lpParam) {
    // Create buffer[] to receive msg
    char buffer[1000] = {0};

    //Create client socket
    SOCKET client_socket = *(SOCKET *)lpParam;

    while(true) {
        int result = recv(client_socket, buffer, sizeof(buffer), 0);
        if(result == SOCKET_ERROR) {
            printf("recv function failed with error %u\n", WSAGetLastError());
            return -1;
        }

        if(strcmp(buffer, "exit") == 0) {
            printf("Client disconnected...\n");
            break;
        }

        printf("Client: %s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    return 1;
}

DWORD WINAPI serverSend(LPVOID lpParam) {
    // Create buffer[] to receive msg
    char buffer[1000] = {0};

    // Create client socket
    SOCKET client_socket = *(SOCKET *)lpParam;

    while(true) {
        gets(buffer);
        int result = send(client_socket, buffer, sizeof(buffer), 0);
        if(result == SOCKET_ERROR) {
            printf("send function failed with error %u\n", WSAGetLastError());
            return -1;
        }

        if(strcmp(buffer, "exit") == 0) {
            printf("Bye client ... Lan sau nho quay lai ...\n");
            break;
        }
    }
    return 1;
}


int main() {
    // Initialize the socket
    WORD wVersionRequested;
    WSADATA wsaData;
    SOCKET server_socket = INVALID_SOCKET;
    struct sockaddr_in service;    // The socket address to be parsed to bind
    int result;

    wVersionRequested = MAKEWORD(2, 2);
    result = WSAStartup(wVersionRequested, &wsaData);  // Return 0 => successful

    if(result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        return 1;
    }

    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1; 
    } else {
        printf("The Winsock 2.2 dll was found OK\n");
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == INVALID_SOCKET) {
        printf("Socket function failed with error: %u\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(PORT);

    // Bind the socket
    result = bind(server_socket, (SOCKADDR *) &service, sizeof(service));
    if(result == SOCKET_ERROR) {
        printf("Bind failed with error %u\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    } else {
        printf("Bind returned successfully\n");
    }
    
    
    // Listen to connection

    result = listen(server_socket, SOMAXCONN);
    if(result == SOCKET_ERROR) {
        printf("Listen function failed with error: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    } else {
        printf("Listening on socket ...\n");
    }

    // Accept connection
    SOCKET client_socket;
    printf("Waiting for client to connect...\n");

    client_socket = accept(server_socket, NULL, NULL);
    if(client_socket == INVALID_SOCKET) {
        printf("Accept failed with error: %ld\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    } else {
        printf("Client connected successfully...\n");
    }
    // Send / Receive msg

    DWORD tid;

    HANDLE t1 = CreateThread(NULL, 0, serverReceive, &client_socket, 0, &tid);
    if(t1 == NULL) {
        printf("Thread creation (serviceReceive) failed with error %u\n", WSAGetLastError());
    }
    HANDLE t2 = CreateThread(NULL, 0, serverSend, &client_socket, 0, &tid);
    if(t2 == NULL) {
        printf("Thread creation (serviceSend) failed with error %u\n", WSAGetLastError());
    }
    WaitForSingleObject(t1, INFINITE);
    WaitForSingleObject(t2, INFINITE);

    // Close socket
    closesocket(server_socket);
    WSACleanup();
    return 0;
}