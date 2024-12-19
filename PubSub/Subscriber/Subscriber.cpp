#include <ws2tcpip.h>
#include "Subscriber.h"
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")  // Veza sa Windows socket library

bool InitializeWindowsSockets() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void PrintTopicsMenu(const char** topics, int topicCount) {
    printf("\nAvailable topics to subscribe:\n");
    for (int i = 0; i < topicCount; i++) {
        printf("%d. %s\n", i + 1, topics[i]);
    }
    printf("%d. Exit\n", topicCount + 1);
}

void ChooseSubscription(SubscriberData* subscriber, const char** topics, int topicCount) {
    int choice;

    // Prikazivanje opcija (tema ili lokacija)
    printf("Choose an option:\n");
    printf("1. Choose a topic\n");
    printf("2. Choose a location\n");
    printf("Enter your choice: ");
    scanf_s("%d", &choice);

    if (choice == 1) {
        // Izbor teme
        int topicChoice;
        do {
            PrintTopicsMenu(topics, topicCount);
            printf("\nEnter your topic choice: ");
            scanf_s("%d", &topicChoice);
            if (topicChoice < 1 || topicChoice > topicCount) {
                printf("Invalid choice, please try again.\n");
            }
        } while (topicChoice < 1 || topicChoice > topicCount);

        // Set the chosen topic
        strcpy_s(subscriber->subscription.topic, topics[topicChoice - 1]);

        // Random location (not used, as user chose a topic)
        subscriber->subscription.location = -1;  // Indicates no location is selected
    }
    else if (choice == 2) {
        // Izbor lokacije
        int location;
        printf("\nEnter the location (0-999): ");
        scanf_s("%d", &location);

        // Set the chosen location
        subscriber->subscription.location = location;

        // Clear topic since location was chosen
        strcpy_s(subscriber->subscription.topic, "");  // No topic selected
    }
    else {
        printf("Invalid choice. Please choose again.\n");
        return ChooseSubscription(subscriber, topics, topicCount); // Recursively call in case of invalid choice
    }

    printf("Enter the start time (format: HH:mm:ss): ");
    scanf_s("%s", subscriber->subscription.startTime, sizeof(subscriber->subscription.startTime));

    printf("Enter the end time (format:  HH:mm:ss): ");
    scanf_s("%s", subscriber->subscription.endTime, sizeof(subscriber->subscription.endTime));

    // Prikazivanje rezultata
    if (subscriber->subscription.location != -1) {
        printf("\nSubscribed to Location: %d\n", subscriber->subscription.location);
    }
    else {
        printf("\nSubscribed to Topic: %s\n", subscriber->subscription.topic);
    }
}


void SendSubscriptionToServer(SubscriberData* subscriber) {
    int bytesSent = send(subscriber->connectSocket,
        (char*)subscriber,
        sizeof(SubscriberData), // Send the whole SubscriberData structure
        0);
    if (bytesSent == SOCKET_ERROR) {
        printf("Failed to send subscription to server.\n");
    }
    else {
        printf("Subscription sent to server successfully.\n");
    }
}




int main() {
    // Inicijalizacija Windows soketa
    if (!InitializeWindowsSockets()) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }
    const char* topics[3] = { "Power", "Voltage", "Strength" };
    SubscriberData subscriber;

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

    subscriber.connectSocket = connectSocket;
    ChooseSubscription(&subscriber, topics, 3);
    SendSubscriptionToServer(&subscriber);



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