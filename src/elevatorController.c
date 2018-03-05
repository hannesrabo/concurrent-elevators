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
#include "targetQueue.h"

void updatePosition(ElevatorStatus* status, double newPosition);

void *ElevatorController(void *elevator_status_arg)
{
	ElevatorStatus *status = (ElevatorStatus *) elevator_status_arg;
	EventQueueItem *nextEvent;

	// Handle events
	while (1)
	{
		nextEvent = event_queue_pop(status->events);

		switch (nextEvent->type)
		{
		case Position:
			if (nextEvent->event->cp.cabin == status->id)
				updatePosition(status, nextEvent->event->cp.position);
			printf("Position received: %f\n", status->position);
			break;

		case Speed:
			status->speed = nextEvent->event->s.speed;
			printf("Speed received: %f\n", status->speed);
			break;

		case CabinButton:
			if (nextEvent->event->cbp.cabin == status->id)
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
			printf("Cart pickup assigned at %d to cart %d\n", nextEvent->event->fbp.floor, status->id);
			break;

		default:
			printf("[ERROR] Elevator controler %d got invalid event code (%d)!", status->id, nextEvent->type);
			exit(1);
			break;

		}

		// This is where all elements are freed
		event_queue_free_element(nextEvent);
	}

	return 0;
}

void updatePosition(ElevatorStatus* status, double newPosition)
{
	status->position = newPosition;
	// This is where we need to handle when to stop the elevators.
	printf("Position received: %f\n", status->position);
}