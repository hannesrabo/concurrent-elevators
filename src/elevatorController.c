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
		nextEvent = event_queue_pop(information->events);

		switch (nextEvent->type)
		{
		case Position:
			if (nextEvent->event->cp.cabin == information->id)
				updatePosition(&position, nextEvent->event->cp.position);
			printf("Position received: %f\n", position);
			break;

		case Speed:
			speed = nextEvent->event->s.speed;
			printf("Speed received: %f\n", speed);
			break;

		case CabinButton:
			if (nextEvent->event->cbp.cabin == information->id)
			{
				int floor = nextEvent->event->cbp.floor;
				if (floor == 32000)
				{
					printf("Cabin stop button pressed in cabin %d\n", nextEvent->event->cbp.cabin);
				}
				else
				{
					printf("Cabin button pressed in cabin %d to floor %d!\n", nextEvent->event->cbp.cabin, floor);
				}
			}
			break;

		case FloorButton:
			printf("Cart pickup assigned at %d to cart %d\n", nextEvent->event->fbp.floor, information->id);
			break;

		default:
			printf("[ERROR] Elevator controler %d got invalid event code (%d)!", information->id, nextEvent->type);
			exit(1);
			break;

		}

		// This is where all elements are freed
		event_queue_free_element(nextEvent);
	}

	return 0;
}

void updatePosition(double *currentPosition, double newPosition)
{
	*currentPosition = newPosition;

	// This is where we need to handle when to stop the elevators.
	printf("Position received: %f\n", *currentPosition);
}