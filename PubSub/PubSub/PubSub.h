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

#define SERVER_PORT "27019" 
#define SUBSCRIBER_PORT "27020"
#define BUFFER_SIZE 100

struct PublisherMessage {
    int location;
    char topic[15];      // Naziv topika
    int message;
    //time_t publicationTime; // Vreme kada je poruka objavljena
    int publicationTime;
};

typedef struct Subscriber {
    int id;
    void* topic;        // Pretplata na topik
    int location;       // Pretplata na lokaciju
} Subscriber;

void Connect();
void subscribeTopic(void* topic);
void subscribeLocation(int location);
void getMessagesTopic(void* startTime, void* endTime);
void getMessagesLocation(void* startTime, void* endTime);
void Publish(void* topic, int location, void* message);
bool InitializeServer();
void CleanupServer();
#endif // PUBLISHER_H









