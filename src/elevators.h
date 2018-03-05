#ifndef __ELEVATORS_H
#define __ELEVATORS_H

#include <pthread.h>

#include "eventQueue.h"
#include "targetQueue.h"

typedef struct
{
	int id;
	pthread_mutex_t sendMutex;
	double speed;
	double position;
	EventQueue *events;
	TargetQueue *q_up;
	TargetQueue *q_down; 
} ElevatorStatus;

#endif