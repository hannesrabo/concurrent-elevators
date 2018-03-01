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

void *ElevatorController(void *argument)
{
	ElevatorInformation *information = (ElevatorInformation *)argument;
	EventQueueItem *nextEvent;

	double speed = 1;
	double position = 0;

	getSpeed();
	whereIs(information->id);

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
		case FloorButton:
		case CabinButton:
		case Error:
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