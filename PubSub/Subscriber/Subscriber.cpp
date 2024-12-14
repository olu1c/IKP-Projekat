#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <conio.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")  // Linkovanje sa Winsock bibliotekom

#define PORT 27020

// Initialize Winsock
bool InitializeWindowsSockets() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return false;
    }
    return true;
}

int main() {
    if (!InitializeWindowsSockets()) {
        return 1;
    }

    SOCKET connectSocket = INVALID_SOCKET;

    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);  // Server port

    // Konvertovanje IP adrese u binarni format
    if (InetPton(AF_INET, L"127.0.0.1", &serverAddress.sin_addr) <= 0) {
        printf("Invalid address.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // Povezivanje na server
    if (connect(connectSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");

    // Čišćenje i zatvaranje socket-a nakon što je povezivanje završeno
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}