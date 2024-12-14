#ifndef PUHLISHER_H
#define PUBLISHER_H
#include <WinSock2.h>
#include <stdbool.h>
#include <time.h>

typedef struct PublisherTopic {
	int location;
	char topic[15];
	int message;
	int publicationTime;

} MeasurementResult;

bool InitializeWindowSocket();
void PrintTopicsMenu(const char** topics, int topicCount);
void ChooseMessage(MeasurementResult* result, const char** topics, int topicCount);
#endif // !PUHLISHER_H

