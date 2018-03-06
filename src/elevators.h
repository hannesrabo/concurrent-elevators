#ifndef __ELEVATORS_H
#define __ELEVATORS_H

#include <pthread.h>
#include <stdbool.h>

#include "eventQueue.h"
#include "targetQueue.h"

typedef enum {
	SweepUp,
	SweepDown,
	SweepIdle
} SweepDirection;

typedef enum {
	MovingUp,
	MovingDown,
	NotMoving
} CurrentMovement;

typedef struct
{
	int id;
	pthread_mutex_t sendMutex;

	// Current
	double speed;
	double position;

	// Current sweep
	SweepDirection sweep_direction;
	CurrentMovement current_movement;
	bool door_opened;

	EventQueue *events;
	TargetQueue *q_up;
	TargetQueue *q_down; 
} ElevatorStatus;

#endif