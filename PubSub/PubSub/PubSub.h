#ifndef PUBLISHER_H
#define PUBLISHER_H
#include <stdbool.h>
#include <time.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

#define SERVER_PORT 27019
#define SERVER1_PORT 27000

#define BUFFER_SIZE 100

struct PublisherMessage {
    int location;
    char topic[15];      // Naziv topika
    int message;
    //time_t publicationTime; // Vreme kada je poruka objavljena
    char publicationTime[20];
};

struct SubscriberRequest {
    int location;               // Lokacija za koju su podaci potrebni
    char topic[20];             // Tema (power, voltage, strength)
    char startTime[20];           // Početno vreme opsega
    char endTime[20];             // Krajnje vreme opsega
};

typedef struct {
    SOCKET connectSocket;
    SubscriberRequest subscription;
} SubscriberData;

typedef struct {
    PublisherMessage buffer[BUFFER_SIZE];
    int head; // Write position
    int tail; // Read position
    int size; // Number of messages in buffer
    CRITICAL_SECTION cs;
} CircularBuffer;

void InitializeCircularBuffer(CircularBuffer* cb);
bool AddMessageToBuffer(CircularBuffer& cb, const PublisherMessage& message);


void Connect();
void subscribeTopic(void* topic);
void subscribeLocation(int location);
void getMessagesTopic(void* startTime, void* endTime);
void getMessagesLocation(void* startTime, void* endTime);
void Publish(void* topic, int location, void* message);
bool InitializeServer();
void CleanupServer();
#endif // PUBLISHER_H









