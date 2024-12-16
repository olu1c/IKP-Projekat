#include <ws2tcpip.h>

#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")  // Veza sa Windows socket library

bool InitializeWindowsSockets() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

int main() {
    // Inicijalizacija Windows soketa
    if (!InitializeWindowsSockets()) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Podešavanje server adrese
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(27000);  // Port na kojem server osluškuje za publishera
    if (InetPton(AF_INET, L"127.0.0.1", &serverAddress.sin_addr) <= 0) { // Povezivanje na localhost
        std::cerr << "Invalid address." << std::endl;
        WSACleanup();
        return 1;
    }

    // Kreiranje socket-a za povezivanje
    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Povezivanje na server
    if (connect(connectSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Unable to connect to server. Error: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Subscriber connected to server." << std::endl;

    // Ovdje možete dodati funkcionalnost za prijem podataka od servera
    char buffer[1024];
    int bytesReceived = recv(connectSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // Dodaj null terminator na primljeni string
        std::cout << "Message from server: " << buffer << std::endl;
    }

    // Zatvaranje socket-a i čišćenje resursa
    closesocket(connectSocket);
    WSACleanup();

    return 0;
}