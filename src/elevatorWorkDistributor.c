#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "elevatorWorkDistributor.h"
#include "elevators.h"

void handleFloorButtonPress(EventQueueItem *eventItem, ElevatorStatus **elevators);

/**
 * This worker thread is handeling computation of paths and work assignment
 * for the different carts.
 */
void *ElevatorWorkDistributor(void *argument)
{
	ElevatorWorkDistributorArgument *ewdarg = (ElevatorWorkDistributorArgument *)argument;
	// int numberOfElevators = ewdarg->numberOfElevators;
	ElevatorStatus **elevators = ewdarg->elevators;
	EventQueue *events = ewdarg->events;
	EventQueueItem *nextEvent;

	while (1)
	{
		nextEvent = event_queue_pop(events);

		switch (nextEvent->type)
		{
		case FloorButton:
			// We need to push the event on the correct queue.
			handleFloorButtonPress(nextEvent, elevators);
			break;
		case CabinButton:
			// This is handled by the cabin itself.
			event_queue_push(elevators[nextEvent->event->cbp.cabin]->events, nextEvent);
			break;
		default:
			printf("[ERROR] Unknown event code in elevator work distributor (%d)!\n", nextEvent->type);
			exit(1);
			break;
		}
	}

	// We do not free the event here as the ownership is passed on.

	return 0;
}

/**
 * Calculates the cost for different carts and then push the event on the 
 * queue with least cost.
 */
void handleFloorButtonPress(EventQueueItem *eventItem, ElevatorStatus **elevators) 
{
	// This is just a test. Push it to the first cart
	event_queue_push(elevators[1]->events, eventItem); 
}