#pragma once
#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <winsock2.h>
#include <stdbool.h>

#define TOPIC_COUNT 3
#define TOPIC_POWER "Power"
#define TOPIC_VOLTAGE "Voltage"
#define TOPIC_STRENGTH "Strength"


struct SubscriberRequest {
    int location;
    char topic[20];
    char startTime[20];
    char endTime[20];
};

typedef struct {
    SOCKET connectSocket;
    SubscriberRequest subscription;
} SubscriberData;

bool InitializeWindowsSockets();
void PrintTopicsMenu(const char** topics, int topicCount);
void PrintTopicsMenu(const char** topics, int topicCount);
void ChooseSubscription(SubscriberData* subscriber, const char** topics, int topicCount);
void SendSubscriptionToServer(SubscriberData* subscriber);


#endif // SUBSCRIBER_H