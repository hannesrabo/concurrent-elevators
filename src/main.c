/*
 * This is the main file for the project
 *
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../hwAPI/hardwareAPI.h"
#include "elevatorController.h"
#include "elevatorWorkDistributor.h"
#include "elevators.h"
#include "eventQueue.h"
#include "masterEventHandler.h"
#include "targetQueue.h"

ElevatorStatus **allocate_elevator_information(int numberOfElevators, int numberOfFloors, pthread_mutex_t sendMutex);

int main(int argc, char *argv[])
{
	char hostname[30];
	int port = 4711;
	int numberOfElevators = 1;
	int numberOfFloors = 5;

	if (argc != 5)
	{
		fprintf(stderr, "Usage: %s host-name port numberOfElevators numberOfFloors\n", argv[0]);
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

	if ((numberOfElevators = atoi(argv[3])) <= 0)
	{
		fprintf(stderr, "Not enough elevators: %s\n", argv[3]);
		fflush(stderr);
		exit(-1);
	}

	if ((numberOfFloors = atoi(argv[4])) <= 0)
	{
		fprintf(stderr, "Not enough floors: %s\n", argv[4]);
		fflush(stderr);
		exit(-1);
	}

	initHW(hostname, port);

	pthread_mutex_t sendMutex;
	pthread_mutex_init(&sendMutex, NULL);

	ElevatorStatus **elevators =
		allocate_elevator_information(numberOfElevators, numberOfFloors, sendMutex);
	pthread_t elevatorControllers[numberOfElevators];
	int i;

	// Create the threads and request initial values.
	pthread_mutex_lock(&sendMutex);
	for (i = 1; i <= numberOfElevators; i++)
	{
		pthread_create(&elevatorControllers[i], NULL, ElevatorController,
					   (void *)elevators[i]);

		// Initial position.
		whereIs(elevators[i]->id);
	}

	// Request an update of the speed of carts
	getSpeed();
	pthread_mutex_unlock(&sendMutex);

	// Create the work distributor and calculation thread
	pthread_t elevatorWorkDistributor;
	ElevatorWorkDistributorArgument ewdarg;
	ewdarg.numberOfElevators = numberOfElevators;
	ewdarg.elevators = elevators;
	ewdarg.sendMutex = sendMutex;
	ewdarg.events = event_queue_create();
	pthread_create(&elevatorWorkDistributor, NULL, ElevatorWorkDistributor, (void *)&ewdarg);

	// Run master event handler.
	masterEventHandler(&ewdarg);

	pthread_join(elevatorWorkDistributor, NULL);
	for (i = 1; i <= numberOfElevators; i++)
	{
		pthread_join(elevatorControllers[i], NULL);
	}

	// This is wrong now...  we need to free them individually.
	free(elevators);
	return 0;
}

ElevatorStatus **allocate_elevator_information(int numberOfElevators, int numberOfFloors, pthread_mutex_t sendMutex)
{
	ElevatorStatus **elevators = (ElevatorStatus **)malloc(sizeof(ElevatorStatus *) * (numberOfElevators + 1));

	int i;
	for (i = 1; i <= numberOfElevators; i++)
	{
		elevators[i] = (ElevatorStatus *)malloc(sizeof(ElevatorStatus));
		printf("Allocated elevator nr: %d\n", i);

		elevators[i]->id = i;
		elevators[i]->sendMutex = sendMutex; // shared
		elevators[i]->events = event_queue_create();
		elevators[i]->q_up = target_queue_create();
		elevators[i]->q_down = target_queue_create();
		elevators[i]->q_down->direction = Down;

		elevators[i]->position = 0;
		elevators[i]->speed = 0;
		elevators[i]->door_action_time = 0;
		elevators[i]->top_floor = numberOfFloors;
		elevators[i]->sweep_direction = SweepIdle;
		elevators[i]->door_status = DoorsClosed;
	}

	printf("All elevators allocated\n");

	return elevators;
}