#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <conio.h>
#include <ctime>
#include "Publisher.h"
#include <Windows.h>
#pragma comment(lib,"WS2_32.lib")


#define BUFFER_SIZE 1024

bool InitializeWindowsSockets() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return false;
    }
    return true;
}

void GenerateRandomMessage(MeasurementResult* result, const char** topics, int topicCount) {
    // Generišite nasumičnu lokaciju (0-999)
    result->location = rand() % 100;

    // Odaberite nasumičnu temu
    strcpy_s(result->topic, topics[rand() % topicCount]);

    // Generišite nasumičnu vrednost poruke
    result->message = rand() % 1000;

    // Dobavljanje trenutnog vremena
    std::time_t now = std::time(nullptr);
    std::tm localTime; // Kreiramo objekat za lokalno vreme

    // Koristimo localtime_s za bezbedno konvertovanje vremena
    if (localtime_s(&localTime, &now) == 0) {
        // Formatiraj trenutno vreme u obliku "hh:mm:ss"
        std::strftime(result->publicationTime, sizeof(result->publicationTime), "%H:%M:%S", &localTime);
    }
    else {
        // U slučaju greške, postavi prazan string
        strcpy_s(result->publicationTime, sizeof(result->publicationTime), "00:00:00");
    }
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
    serverAddress.sin_port = htons(27019);

    if (InetPton(AF_INET, L"127.0.0.1", &serverAddress.sin_addr) <= 0) {
        printf("Invalid address.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    if (connect(connectSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");
    const char* topics[3] = { "Power", "Voltage", "Strength" };

    
    // Petlja za slanje poruka
    for (int i = 0; i < 10000; i++) {
        MeasurementResult res;
        GenerateRandomMessage(&res, topics, 3);

        // Slanje poruke serveru
        int bytesSent = send(connectSocket, (char*)&res, sizeof(MeasurementResult), 0);
        if (bytesSent == SOCKET_ERROR) {
            printf("Failed to send message %d to server. Error: %d\n", i + 1, WSAGetLastError());
        }
        else {
            printf("Message %d sent: Location=%d, Topic=%s, Value=%d, Time=%s\n",
                i + 1, res.location, res.topic, res.message, res.publicationTime);
        }

        // Pauza od 2 sekunde pre slanja sledeće poruke
        Sleep(2000);
    }

    // Čišćenje
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}

