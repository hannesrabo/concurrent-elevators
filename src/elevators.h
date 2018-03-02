#ifndef __ELEVATORS_H
#define __ELEVATORS_H

#include <pthread.h>

#include "eventQueue.h"

typedef struct
{
	int id;
	pthread_mutex_t sendMutex;
	EventQueue events;
} ElevatorInformation;

#endif