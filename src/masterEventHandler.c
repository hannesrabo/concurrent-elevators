#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>

#include "elevators.h"
#include "elevatorWorkDistributor.h"
#include "../hwAPI/hardwareAPI.h"

void masterEventHandler(elevatorWorkDistributorArgument *ewdarg)
{
	int numberOfElevators = ewdarg->numberOfElevators;
	ElevatorInformation *elevators = ewdarg->elevators;
	EventQueue *workDistributorEvents = &ewdarg->events;

	EventType e;
	EventDesc ed;

	int i;
	while (1)
	{
		e = waitForEvent(&ed);
		switch (e)
		{
		case FloorButton:
			event_queue_push(workDistributorEvents, FloorButton, &ed);
			break;

		case CabinButton:
			event_queue_push(&elevators[ed.cbp.cabin].events, CabinButton, &ed);
			break;

		case Position:
			event_queue_push(&elevators[ed.cp.cabin].events, Position, &ed);
			break;

		case Speed:
			for (i = 0; i < numberOfElevators; i++)
				event_queue_push(&elevators[i].events, Speed, &ed);
			break;

		case Error:
			printf("Error in master event handler!\n");
			break;
		}
	}
}