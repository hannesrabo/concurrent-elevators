#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "elevatorWorkDistributor.h"
#include "elevators.h"

void *ElevatorWorkDistributor(void *argument)
{
	elevatorWorkDistributorArgument *ewdarg = (elevatorWorkDistributorArgument *)argument;
	// int numberOfElevators = ewdarg->numberOfElevators;
	ElevatorInformation *elevators = ewdarg->elevators;
	EventQueue *events = &ewdarg->events;
	EventQueueItem *nextEvent;

	while (1)
	{
		nextEvent = event_queue_front(events);

		switch (nextEvent->type)
		{
		case FloorButton:
			printf("Floor button pressed on floor %d!\n", nextEvent->event->fbp.floor);
			break;
		case CabinButton:
			event_queue_push(&elevators[nextEvent->event->cbp.cabin].events, CabinButton, nextEvent->event);
			break;
		case Position:
		case Speed:
		case Error:
		default:
			printf("Error in Elevator Work Distributor!\n");
			break;
		}
	}

	return 0;
}