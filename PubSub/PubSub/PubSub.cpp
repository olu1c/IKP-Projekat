#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "PubSub.h"
#pragma comment(lib, "Ws2_32.lib")

CircularBuffer cb;
HANDLE emptySemaphore;
HANDLE fullSemaphore;


bool InitializeWindowsSockets() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

void ReceiveMessage(SOCKET clientSocket) {
    PublisherMessage message;
    int bytesReceived;

    while (true) {
        bytesReceived = recv(clientSocket, (char*)&message, sizeof(PublisherMessage), 0);
        if (bytesReceived > 0) {
            if (bytesReceived == sizeof(PublisherMessage)) {
                WaitForSingleObject(emptySemaphore, INFINITE);
                // Dodajte poruku u bafer
                if (AddMessageToBuffer(&cb,&message)) {
                    std::cout << "Message added to buffer:\n";
                    std::cout << "  Location: " << message.location << "\n";
                    std::cout << "  Topic: " << message.topic << "\n";
                    std::cout << "  Value: " << message.message << "\n";
                    std::cout << "  Publication Time: " << message.publicationTime << "\n";
                }
                ReleaseSemaphore(fullSemaphore, 1, NULL); // Signal a new message in buffer

            }
            else {
                std::cout << "Received message size does not match expected size.\n";
            }
        }
        else if (bytesReceived == 0) {
            // Publisher se isključio
            std::cout << "Publisher disconnected.\n";
            break;
        }
        else {
            std::cerr << "Error receiving message: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // Zatvaranje konekcije sa publisher-om
    closesocket(clientSocket);
}



void ReceiveSubscriptionFromClient(SOCKET clientSocket) {
    //SubscriberData subscriber;
    SubscriberData* subscriber = (SubscriberData*)malloc(sizeof(SubscriberData));

    // Prihvatanje podataka o pretplati od klijenta
    int bytesReceived = recv(clientSocket, (char*)subscriber, sizeof(SubscriberData), 0);
    if (bytesReceived == sizeof(SubscriberData)) {
        subscriber->connectSocket = clientSocket;

        if (subscriber->subscription.location != -1) {
            // Pretplata na lokaciju
            printf("Korisnik se pretplatio na lokaciju: %d\n", subscriber->subscription.location);
        }
        else if (subscriber->subscription.topic[0] != '\0') {
            // Pretplata na temu (pretpostavljamo da je 'topic' string)
            printf("Korisnik se pretplatio na temu: %s\n", subscriber->subscription.topic);
        }
        else {
            printf("Korisnik nije pretplaćen na ni jednu temu ni lokaciju.\n");
        }

        // Provera unetog vremena (ako je potrebno)
        if (subscriber->subscription.startTime[0] != '\0') {
            printf("Pocetno vreme: %s\n", subscriber->subscription.startTime);
        }
        else {
            printf("Pocetno vreme nije uneto.\n");
        }

        if (subscriber->subscription.endTime[0] != '\0') {
            printf("Krajnje vreme: %s\n", subscriber->subscription.endTime);
        }
        else {
            printf("Krajnje vreme nije uneto.\n");
        }

        // Dodavanje pretplatnika na odgovarajuću temu ili lokaciju
        //AddSubscriberToTopic(subscriber->subscription.topic, subscriber);
        if (subscriber->subscription.location != -1) {
            subscribeLocation(subscriber->subscription.location, subscriber);
            getMessagesLocation(subscriber->subscription.startTime, subscriber->subscription.endTime);

        }
        else {
            subscribeTopic(subscriber->subscription.topic, subscriber);
            getMessagesTopic(subscriber->subscription.startTime, subscriber->subscription.endTime);

        }

       

    }
    else {
        printf("Failed to receive");
    }
    
   
}

bool isTimeInRange(const char* publicationTime, const char* startTime, const char* endTime) {
    if (strcmp(publicationTime, startTime) >= 0 && strcmp(publicationTime, endTime) <= 0) {
        printf("Checking if message time: %s is in range [%s - %s]\n",publicationTime, startTime, endTime);

        return true;
    }
    return false;
}




void InitializeCircularBuffer(CircularBuffer* cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->size = 0;
    InitializeCriticalSection(&cb->cs);
}

bool AddMessageToBuffer(CircularBuffer* cb, PublisherMessage* message) {
    EnterCriticalSection(&cb->cs);

    // Provera da li je bafer pun
    if (cb->size < BUFFER_SIZE) {
        cb->buffer[cb->head] = *message;
        cb->head = (cb->head + 1) % BUFFER_SIZE;  // Krug kroz bafer
        cb->size++;

        std::cout << "Message added to buffer: " << message->message << std::endl;
        std::cout << "Buffer size: " << cb->size << std::endl; // Prikazivanje trenutne veličine bafera

        LeaveCriticalSection(&cb->cs);
        return true;
    }

    LeaveCriticalSection(&cb->cs);
    std::cout << "Buffer is full, cannot add message." << std::endl;
    return false; // Bafer je pun
}

void getMessagesTopic( const char* startTime, const char* endTime) {
    // Prolazite kroz sve poruke u bufferu

    
    WaitForSingleObject(fullSemaphore, INFINITE);
  

    EnterCriticalSection(&cb.cs);
   

    printf("Veličina bafera: %d\n", cb.size);


    for (int i = 0; i < cb.size; ++i) {
        PublisherMessage& message = cb.buffer[(cb.tail + i) % BUFFER_SIZE];

        // Proverite da li je poruka unutar vremenskog intervala
        if (isTimeInRange(message.publicationTime, startTime, endTime)) {
            printf("Poruka sa temom %s i lokacijom %d u vremenskom intervalu\n", message.topic, message.location);

            // Nađite odgovarajuće pretplatnike na osnovu teme
            int index = (strcmp(message.topic, "Power") == 0) ? 0 : (strcmp(message.topic, "Voltage") == 0) ? 1 : 2;

            // Pristupite odgovarajućem HashmapEntry za temu
            HashmapEntry* entry = &topicSubscribers[index];

            char formattedMessage[100];
            snprintf(formattedMessage, sizeof(formattedMessage),
                "Location: %d, Topic: %s, Message: %d", message.location, message.topic, message.message);

            // Ispis broja pretplatnika za temu
            printf("Broj pretplatnika za temu %s: %d\n", message.topic, entry->subscriberCount);

            // Pošaljite poruku svim pretplatnicima za temu
            for (int j = 0; j < entry->subscriberCount; ++j) {
                SubscriberData* subscriber = entry->subscribers[j];

                // Ispis koji pretplatnik prima poruku
                printf("Slanje poruke pretplatniku na socketu: %d\n", subscriber->connectSocket);

                // Pošaljite poruku pretplatniku
                send(subscriber->connectSocket, formattedMessage, strlen(formattedMessage), 0);
            }
        }
    }


    // Ispis da je funkcija završila sa radom
    printf("Obrada poruka završena.\n");

    LeaveCriticalSection(&cb.cs);
    ReleaseSemaphore(emptySemaphore, 1, NULL);

}

void getMessagesLocation(const char* startTime, const char* endTime) {
    // Prolazite kroz sve poruke u bufferu

    printf("Početak obrade poruka sa lokacijama između %s i %s...\n", startTime, endTime);

   WaitForSingleObject(fullSemaphore, INFINITE);

    EnterCriticalSection(&cb.cs);

    for (int i = 0; i < cb.size; ++i) {
        PublisherMessage& message = cb.buffer[(cb.tail + i) % BUFFER_SIZE];

        // Proverite da li je poruka unutar vremenskog intervala
        if (isTimeInRange(message.publicationTime, startTime, endTime)) {
            printf("Obrada poruke: Tema = %s, Lokacija = %d, Poruka = %d, Vreme = %s\n",
                message.topic, message.location, message.message, message.publicationTime);

            // Nađite odgovarajuće pretplatnike na osnovu lokacije
            int index = message.location; // Pretpostavljamo da je 'location' direktan indeks u hashmapi

            // Pristupite odgovarajućem HashmapEntry za lokaciju
            if (index >= 0 && index <=999) {
                HashmapEntry* entry = &locationSubscribers[index];

                char formattedMessage[100];
                snprintf(formattedMessage, sizeof(formattedMessage),
                    "Location: %d, Topic: %s, Message: %d", message.location, message.topic, message.message);

                // Ispis broja pretplatnika za lokaciju
                printf("Broj pretplatnika za lokaciju %d: %d\n", message.location, entry->subscriberCount);

                // Pošaljite poruku svim pretplatnicima za lokaciju
                for (int j = 0; j < entry->subscriberCount; ++j) {
                    SubscriberData* subscriber = entry->subscribers[j];

                    // Ispis koji pretplatnik prima poruku
                    printf("Slanje poruke pretplatniku sa socketom: %d, Tema: %s, Lokacija: %d\n",
                        subscriber->connectSocket, message.topic, message.location);

                    // Pošaljite poruku pretplatniku
                    int sendResult = send(subscriber->connectSocket, formattedMessage, strlen(formattedMessage), 0);
                    if (sendResult == SOCKET_ERROR) {
                        printf("Greška prilikom slanja poruke na socket %d\n", subscriber->connectSocket);
                    }
                    else {
                        printf("Poruka uspešno poslata pretplatniku na socketu: %d\n", subscriber->connectSocket);
                    }
                }
            }
            else {
                printf("Nema pretplatnika za lokaciju %d.\n", message.location);
            }
        }
    }

    // Ispis da je funkcija završila sa radom
    printf("Obrada poruka završena.\n");

    LeaveCriticalSection(&cb.cs);
    ReleaseSemaphore(emptySemaphore, 1, NULL);
}



DWORD WINAPI PublisherThread(LPVOID lpParam) {
    SOCKET publisherSocket = *(SOCKET*)lpParam;

    std::cout << "Publisher thread started.\n";
    ReceiveMessage(publisherSocket);

    return 0;
}

void InitializeHashmaps() {
    memset(locationSubscribers, 0, sizeof(locationSubscribers));
    memset(topicSubscribers, 0, sizeof(topicSubscribers));
}



void subscribeTopic(const char* topic, SubscriberData* subscriber) {
    //printf("AddSubscriberToTopic pozvan za topik: %s\n", subscriber->subscription.topic);
    

    int index = (strcmp(topic, "Power") == 0) ? 0 : (strcmp(topic, "Voltage") == 0) ? 1 : 2;
    HashmapEntry* entry = &topicSubscribers[index];
    if (entry->subscriberCount < 10) {
        entry->subscribers[entry->subscriberCount++] = subscriber;
        printf("Pretplatnik dodat u mapu");

        //printf("Dodavanje pretplatnika na adresi %p za temu %s\n", (void*)subscriber, topic);
        //for (int i = 0; i < entry->subscriberCount; ++i) {
            //printf("Pretplatnik %d na adresi %p: %s\n", i + 1, (void*)entry->subscribers[i], entry->subscribers[i]->subscription.topic);
        //}
    }
    
}

void subscribeLocation(int location, SubscriberData* subscriber) {
    HashmapEntry* entry = &locationSubscribers[location];
    if (entry->subscriberCount < 10) {
        entry->subscribers[entry->subscriberCount++] = subscriber;
        printf("Pretplatnik dodat u mapu");

        //printf("Dodavanje pretplatnika na adresi %p na lokaciju %d\n", (void*)subscriber, location);
        //for (int i = 0; i < entry->subscriberCount; ++i) {
            //printf("Pretplatnik %d na adresi %p: %d\n", i + 1, (void*)entry->subscribers[i], entry->subscribers[i]->subscription.location);
        //}
    }
}


DWORD WINAPI SubscriberThread(LPVOID lpParam) {
    SOCKET subscriberSocket = *(SOCKET*)lpParam;
    //CircularBuffer* cb = (CircularBuffer*)lpParam;


    std::cout << "Subscriber thread started.\n";
    ReceiveSubscriptionFromClient(subscriberSocket);

    return 0;
}



int main() {
    if (!InitializeWindowsSockets()) {
        return 1;
    }

    
    InitializeCircularBuffer(&cb);

    // Postavljanje adresa i portova
    sockaddr_in publisherAddress;
    publisherAddress.sin_family = AF_INET;
    publisherAddress.sin_addr.s_addr = INADDR_ANY;
    publisherAddress.sin_port = htons(SERVER_PORT);

    sockaddr_in subscriberAddress;
    subscriberAddress.sin_family = AF_INET;
    subscriberAddress.sin_addr.s_addr = INADDR_ANY;
    subscriberAddress.sin_port = htons(SERVER1_PORT);

    emptySemaphore = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
    fullSemaphore = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
    // Kreiranje socket-a za publisher-a i subscriber-a
    SOCKET publisherListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKET subscriberListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (publisherListenSocket == INVALID_SOCKET || subscriberListenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind za publisher i subscriber socket
    if (bind(publisherListenSocket, (sockaddr*)&publisherAddress, sizeof(publisherAddress)) == SOCKET_ERROR) {
        std::cerr << "Publisher bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(publisherListenSocket);
        WSACleanup();
        return 1;
    }

    if (bind(subscriberListenSocket, (sockaddr*)&subscriberAddress, sizeof(subscriberAddress)) == SOCKET_ERROR) {
        std::cerr << "Subscriber bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(subscriberListenSocket);
        WSACleanup();
        return 1;
    }

    // Postavljanje socket-a u režim slušanja
    if (listen(publisherListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen for publisher failed with error: " << WSAGetLastError() << std::endl;
        closesocket(publisherListenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(subscriberListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen for subscriber failed with error: " << WSAGetLastError() << std::endl;
        closesocket(subscriberListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening for publishers on port: " << SERVER_PORT << std::endl;
    std::cout << "Server is listening for subscribers on port: " << SERVER1_PORT << std::endl;


    

    // Prihvatanje konekcija
    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(publisherListenSocket, &readfds);
        FD_SET(subscriberListenSocket, &readfds);

        int maxSocket = (publisherListenSocket > subscriberListenSocket) ? publisherListenSocket : subscriberListenSocket;
        int activity = select(maxSocket + 1, &readfds, NULL, NULL, NULL);

        if (activity == SOCKET_ERROR) {
            std::cerr << "Select failed with error: " << WSAGetLastError() << std::endl;
            break;
        }

        // Provera konekcije za publisher
        if (FD_ISSET(publisherListenSocket, &readfds)) {
            SOCKET publisherSocket = accept(publisherListenSocket, NULL, NULL);
            if (publisherSocket != INVALID_SOCKET) {
                std::cout << "Publisher connected." << std::endl;

                // Kreiranje niti za publisher
                CreateThread(NULL, 0, PublisherThread, (LPVOID)&publisherSocket, 0, NULL);
            }
        }

        // Provera konekcije za subscriber
        if (FD_ISSET(subscriberListenSocket, &readfds)) {
            SOCKET subscriberSocket = accept(subscriberListenSocket, NULL, NULL);
            if (subscriberSocket != INVALID_SOCKET) {
                std::cout << "Subscriber connected." << std::endl;

                // Kreiranje niti za subscriber
                CreateThread(NULL, 0, SubscriberThread, (LPVOID)&subscriberSocket, 0, NULL);
            }
        }
    }



    

    closesocket(publisherListenSocket);
    closesocket(subscriberListenSocket);
    CloseHandle(emptySemaphore);
    CloseHandle(fullSemaphore);
    DeleteCriticalSection(&cb.cs);
    WSACleanup();
    return 0;
}