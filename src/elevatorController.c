#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "../hwAPI/hardwareAPI.h"
#include "elevatorController.h"
#include "elevators.h"

void updatePosition(double *currentPosition, double newPosition);

void *ElevatorController(void *argument)
{
	ElevatorInformation *information = (ElevatorInformation *)argument;
	EventQueueItem *nextEvent;

	double speed = 1;
	double position = 0;

	pthread_mutex_lock(&information->sendMutex);
	getSpeed();
	whereIs(information->id);
	pthread_mutex_unlock(&information->sendMutex);

	while (1)
	{
		nextEvent = event_queue_front(&information->events);

		switch (nextEvent->type)
		{
		case Position:
			if (nextEvent->event->cp.cabin == information->id)
				updatePosition(&position, nextEvent->event->cp.position);
			break;
		case Speed:
			speed = nextEvent->event->s.speed;
			printf("Speed received: %f\n", speed);
			break;
		case CabinButton:
			if (nextEvent->event->cbp.cabin == information->id)
			{
				printf("Cabin button pressed in cabin %d to floor %d!\n", nextEvent->event->cbp.cabin, nextEvent->event->cbp.floor);
			}
			break;
		case FloorButton:
		case Error:
		default:
			printf("I %d got error!", information->id);
			break;
		}

		event_queue_pop(&information->events);
	}

	return 0;
}

void updatePosition(double *currentPosition, double newPosition)
{
	*currentPosition = newPosition;
	printf("Position received: %f\n", *currentPosition);
}