#include<windows.h>
#include<winsock2.h>
#include<winsock.h>
#include<ws2tcpip.h>
#include<stdio.h>
#include<string.h>

#define PORT 8080

#pragma comment(lib, "Ws2_32.lib")

DWORD WINAPI clientReceive(LPVOID lpParam) {
    // Create buffer[] to receive msg
    char buffer[1000] = {0};

    // Create server socket
    SOCKET server_socket = *(SOCKET *)lpParam;
    
    while(true) {
        int result = recv(server_socket, buffer, sizeof(buffer), 0);
        if(result == SOCKET_ERROR) {
            printf("recv function failed with error %u\n", WSAGetLastError());
            return -1;
        }
        if(strcmp(buffer, "exit") == 0) {
            printf("Server disconnected...\n");
            return 1;
        }
        printf("Server: %s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    return 1;
}

DWORD WINAPI clientSend(LPVOID lpParam) {
    // Create buffer[] to send msg
    char buffer[1000] = {0};

    // Create server socket
    SOCKET server_socket = *(SOCKET *)lpParam;

    while(true) {
        gets(buffer);
        int result = send(server_socket, buffer, sizeof(buffer), 0);
        if(result == SOCKET_ERROR) {
            printf("send function failed with error %u\n", WSAGetLastError());
            return -1;
        }
        if(strcmp(buffer, "exit") == 0) {
            printf("Bye server ... Sau tao quay lai");
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
    struct sockaddr_in addr;    
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


    // Connect to server 
    
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    result = connect(server_socket, (SOCKADDR *) &addr, sizeof(addr));
    if(result == SOCKET_ERROR) {
        printf("Server connection failed with error: %u\n", WSAGetLastError());
        return -1;
    }

    //If connected successfully
    printf("Connected to server successfully...\n");
    printf("Lets chat with server \n");
    printf("Enter \"exit\" to disconnect\n");

    // Send / Receive msg

    DWORD tid;
    
    HANDLE t1 = CreateThread(NULL, 0, clientReceive, &server_socket, 0, &tid);
    if(t1 == NULL) {
        printf("Thread creation error (clientReceive): %u\n", WSAGetLastError());
    }

    HANDLE t2 = CreateThread(NULL, 0, clientSend, &server_socket, 0, &tid);
    if(t2 == NULL) {
        printf("Thread creation error (clientReceive): %u\n", WSAGetLastError());
    }

    WaitForSingleObject(t1, INFINITE);
    WaitForSingleObject(t2, INFINITE);

    // Close socket
    closesocket(server_socket);
    WSACleanup();
    return 0;
}