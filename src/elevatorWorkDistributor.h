#ifndef __ELEVATOR_WORK_DISTRIBUTOR_H
#define __ELEVATOR_WORK_DISTRIBUTOR_H

#include <pthread.h>

#include "eventQueue.h"
#include "elevators.h"

typedef struct
{
	ElevatorStatus **elevators;
	EventQueue *events;
	int numberOfElevators;
	pthread_mutex_t sendMutex;
} ElevatorWorkDistributorArgument;

void *ElevatorWorkDistributor(void *argument);

#endif