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
#include <pthread.h>

#include "eventQueue.h"
#include "elevators.h"
#include "elevatorController.h"
#include "elevatorWorkDistributor.h"
#include "masterEventHandler.h"
#include "../hwAPI/hardwareAPI.h"

ElevatorInformation *createElevators(int numberOfElevators, pthread_mutex_t sendMutex);

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
	pthread_mutex_t sendMutex;
	pthread_mutex_init(&sendMutex, NULL);

	ElevatorInformation *elevators = createElevators(numberOfElevators, sendMutex);
	pthread_t elevatorControllers[numberOfElevators];
	int i;
	for (i = 1; i <= numberOfElevators; i++)
	{
		pthread_create(&elevatorControllers[i], NULL, ElevatorController, (void *)&elevators[i]);
	}

	pthread_t elevatorWorkDistributor;
	elevatorWorkDistributorArgument ewdarg;
	ewdarg.numberOfElevators = numberOfElevators;
	ewdarg.elevators = elevators;
	ewdarg.sendMutex = sendMutex;
	event_queue_init(&ewdarg.events, numberOfElevators + 1);
	pthread_create(&elevatorWorkDistributor, NULL, ElevatorWorkDistributor, (void *)&ewdarg);

	masterEventHandler(&ewdarg);

	pthread_join(elevatorWorkDistributor, NULL);
	for (i = 1; i <= numberOfElevators; i++)
	{
		pthread_join(elevatorControllers[i], NULL);
	}

	free(elevators);
	return 0;
}

ElevatorInformation *createElevators(int numberOfElevators, pthread_mutex_t sendMutex)
{
	ElevatorInformation *elevators = malloc(sizeof(ElevatorInformation) * numberOfElevators);
	int i;
	for (i = 1; i <= numberOfElevators; i++)
	{
		elevators[i].id = i;
		elevators[i].sendMutex = sendMutex;
		event_queue_init(&(elevators[i].events), i);
	}

	return elevators;
}