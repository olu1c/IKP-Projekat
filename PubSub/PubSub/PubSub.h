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
#define THREAD_POOL 4
#define BUFFER_SIZE 10024

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

typedef struct HashmapEntry {
    SubscriberData* subscribers[10];
    int subscriberCount;
} HashmapEntry;

HashmapEntry locationSubscribers[1000];  // Keys 0-999
HashmapEntry topicSubscribers[3];        // Keys "Power", "Voltage", "Strength"

void InitializeCircularBuffer(CircularBuffer* cb);
bool AddMessageToBuffer(CircularBuffer* cb, PublisherMessage* message);


void Connect();
void InitializeHashmaps();
void subscribeLocation(int location, SubscriberData* subscriber);
void subscribeTopic(const char* topic, SubscriberData* subscriber);

void getMessagesTopic(const char* startTime, const char* endTime);
void getMessagesLocation(const char* startTime,const char* endTime);
void Publish(void* topic, int location, void* message);
bool InitializeServer();
void CleanupServer();
#endif // PUBLISHER_H









