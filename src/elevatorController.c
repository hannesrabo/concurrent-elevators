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

	while (1)
	{
		nextEvent = event_queue_front(&information->events);
		event_queue_pop(&information->events);

		if (nextEvent->type == Position)
		{
			// printf("Error!\n");
			// printf("Error! %s\n", nextEvent->event->e.str);
			printf("Position %f received\n", nextEvent->event->cp.position);
		}
		else
		{
			printf("Next event received!\n");
		}

		sleep(3);
	}

	return 0;
}