#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// #define mem_test
#ifdef mem_test
#include <string.h>
#include <sys/resource.h>
#endif

#include "elevators.h"
#include "elevatorWorkDistributor.h"
#include "../hwAPI/hardwareAPI.h"

void masterEventHandler(ElevatorWorkDistributorArgument *ewdarg)
{
	int numberOfElevators = ewdarg->numberOfElevators;
	ElevatorStatus **elevators = ewdarg->elevators;
	EventQueue *workDistributorEvents = ewdarg->events;

	int i;
	while (1)
	{
		// Allocate new item element. Wait for a new event to be received.
		EventQueueItem *item = event_queue_create_element(0, NULL);
		item->event = (EventDesc *)malloc(sizeof(EventDesc));
		item->type = waitForEvent(item->event);

#ifdef mem_test
		int aa = 0;
		while (aa < 100)
		{
			aa++;

			if (aa == 99)
			{
				struct rusage r_usage;
				getrusage(RUSAGE_SELF, &r_usage);
				printf("Memory usage: %ld kilobytes\n", r_usage.ru_maxrss);
			}
			EventQueueItem *tmp = event_queue_create_element(0, NULL);
			tmp->event = malloc(sizeof(EventDesc));
			memcpy(tmp->event, item->event, sizeof(EventDesc));
			tmp->type = item->type;
			item = tmp;
#endif

			switch (item->type)
			{
			case FloorButton:
				event_queue_push(workDistributorEvents, item);
				break;

			case CabinButton:
				event_queue_push(elevators[item->event->cbp.cabin]->events, item);
				break;

			case Position:
				event_queue_push(elevators[item->event->cp.cabin]->events, item);
				break;

			case Speed:
				for (i = 1; i <= numberOfElevators; i++)
					event_queue_push(elevators[i]->events, item);
				break;

			case Error:
				printf("[ERROR]: Master event handler received error!\n");
				printf("[ERROR] Exiting program\n");
				exit(1);
				break;
			}
#ifdef mem_test
		}
#endif
		// Ownership of the allocated space is passed on the the queue owner.
	}
}