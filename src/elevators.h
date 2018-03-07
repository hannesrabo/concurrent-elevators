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

typedef enum {
	DoorsClosing,
	DoorsOpening,
	DoorsClosed,
	DoorsOpen
} DoorStatus;

typedef struct
{
	int id;
	pthread_mutex_t sendMutex;

	// Current
	double speed;
	double position;

	// Current sweep
	SweepDirection sweep_direction;
	DoorStatus door_status;
	double door_action_time;

	// Elevator
	EventQueue *events;
	TargetQueue *q_up;
	TargetQueue *q_down;
	int top_floor;
} ElevatorStatus;

#endif