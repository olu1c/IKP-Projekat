#pragma comment(lib, "ws2_32.lib") 
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <ctime>
#include "PubSub.h"
#define SERVER_PORT "27019"  // Port servera
#define BUFFER_SIZE 100




// Funkcija koja inicijalizuje server
bool InitializeWindowsSockets() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

// Funkcija koja prihvata konekciju i prima poruku
void ReceiveMessage(SOCKET& clientSocket) {
    PublisherMessage message;
    int bytesReceived;

    while (true) {
        bytesReceived = recv(clientSocket, (char*)&message, sizeof(PublisherMessage), 0);
        if (bytesReceived > 0) {
            if (bytesReceived == sizeof(PublisherMessage)) {

                
                    std::cout << "Added message to buffer:\n";
                    std::cout << "  Location: " << message.location << "\n";
                    std::cout << "  Topic: " << message.topic << "\n";
                    std::cout << "  Value: " << message.message << "\n";
                    std::cout << "  Expiration Time: " << message.publicationTime << "\n";
                

            }
        }
        else {
            std::cout << "Publisher disconnected.\n";
            break;
        }
    }

    closesocket(clientSocket);

   
}

// Funkcija koja zatvara server
void CleanupServer(SOCKET& listenSocket) {
    closesocket(listenSocket);
    WSACleanup();
}

int main() {

    // Inicijalizacija servera
    if (!InitializeWindowsSockets()) {
        return 1;
    }

    sockaddr_in publisherAddress;
    publisherAddress.sin_family = AF_INET;
    publisherAddress.sin_addr.s_addr = INADDR_ANY;
    publisherAddress.sin_port = htons(atoi(SERVER_PORT));

    SOCKET publisherListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


    //Pitamo za gresku kod socketa
    if (publisherListenSocket == INVALID_SOCKET ) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind publisher socket
    if (bind(publisherListenSocket, (sockaddr*)&publisherAddress, sizeof(publisherAddress)) == SOCKET_ERROR) {
        std::cerr << "Publisher bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(publisherListenSocket);
        WSACleanup();
        return 1;
    }

    //Listen kod publishera
    if (listen(publisherListenSocket, SOMAXCONN) == SOCKET_ERROR )
        /*listen(subscriberListenSocket, SOMAXCONN) == SOCKET_ERROR)*/ {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(publisherListenSocket);
        //closesocket(subscriberListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening for publishers on port: " << SERVER_PORT << std::endl;
  
    SOCKET publisherSocket = accept(publisherListenSocket, NULL, NULL);
    if (publisherSocket != INVALID_SOCKET) {
        std::cout << "Publisher connected.\n";
        ReceiveMessage(publisherSocket);
    }
    // Zatvaranje servera nakon što je završen rad
    closesocket(publisherListenSocket);
    WSACleanup();
    return 0;
}