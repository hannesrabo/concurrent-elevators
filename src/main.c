/*
 * This is the main file for the project
 *
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "eventQueue.h"
#include "elevators.h"
#include "elevatorController.h"
#include "elevatorWorkDistributor.h"
#include "../hwAPI/hardwareAPI.h"

ElevatorInformation *createElevators(int numberOfElevators);

int main(int argc, char *argv[])
{
	char hostname[30];
	int port = 4711;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s host-name port\n", argv[0]);
		fflush(stderr);
		exit(-1);
	}
	strcpy(hostname, argv[1]);

	if ((port = atoi(argv[2])) <= 0)
	{
		fprintf(stderr, "Bad port number: %s\n", argv[2]);
		fflush(stderr);
		exit(-1);
	}

	initHW(hostname, port);

	int numberOfElevators = 1;

	ElevatorInformation *elevators = createElevators(numberOfElevators);
	pthread_t elevatorControllers[numberOfElevators];
	int i;
	for (i = 0; i < numberOfElevators; i++)
	{
		pthread_create(&elevatorControllers[i], NULL, ElevatorController, (void *)&elevators[i]);
	}

	pthread_t elevatorWorkDistributor;
	pthread_create(&elevatorWorkDistributor, NULL, ElevatorWorkDistributor, (void *)elevators);

	pthread_join(elevatorWorkDistributor, NULL);
	for (i = 0; i < numberOfElevators; i++)
	{
		pthread_join(elevatorControllers[i], NULL);
	}

	free(elevators);
	return 0;
}

ElevatorInformation *createElevators(int numberOfElevators)
{
	ElevatorInformation *elevators = malloc(sizeof(ElevatorInformation) * numberOfElevators);
	int i;
	for (i = 0; i < numberOfElevators; i++)
	{
		elevators[i].id = i;
		event_queue_init(&(elevators[i].events), i);
	}

	return elevators;
}