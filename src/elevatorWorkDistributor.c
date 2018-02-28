#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <string.h>
#include "elevators.h"

void *ElevatorWorkDistributor(void *argument)
{
	ElevatorInformation *elevators = (ElevatorInformation *)argument;

	EventType type = Position;
	EventDesc *desc = malloc(sizeof(EventDesc));
	desc->cp.cabin = 0;
	desc->cp.position = 1.5;

	event_queue_push(&elevators[0].events, type, desc);

	free(desc);

	return 0;
}