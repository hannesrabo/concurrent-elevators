/*
 * This is the main file for the project
 *
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>

#include "eventQueue.h"

typedef struct
{
	int id;
	EventQueue events;
} ElevatorInformation;

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s host-name port\n", argv[0]);
		fflush(stderr);
		exit(-1);
	}
	hostname = argv[1];
	if ((port = atoi(argv[2])) <= 0)
	{
		fprintf(stderr, "Bad port number: %s\n", argv[2]);
		fflush(stderr);
		exit(-1);
	}

	initHW(hostname, port);

	int numberOfElevators = 1;
	ElevatorInformation *elevators = createElevators(1);

	free(elevatorList);
	return 0;
}

elevatorInformation *createElevators(int numberOfElevators)
{
	ElevatorInformation *elevators = malloc(sizeof(elevatorInformation) * numberOfElevators);
	int i;
	for (i = 0; i < numberOfElevators; i++)
	{
		elevators[i]->id = i;
		event_queue_init(elecators[i]->events);
	}

	return elevators;
}