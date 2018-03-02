#ifndef __ELEVATOR_WORK_DISTRIBUTOR_H
#define __ELEVATOR_WORK_DISTRIBUTOR_H

#include <pthread.h>

#include "eventQueue.h"
#include "elevators.h"

typedef struct
{
	ElevatorInformation *elevators;
	EventQueue events;
	int numberOfElevators;
	pthread_mutex_t sendMutex;
} elevatorWorkDistributorArgument;

void *ElevatorWorkDistributor(void *argument);

#endif