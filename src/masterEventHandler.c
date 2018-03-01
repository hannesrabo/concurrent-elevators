#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>

#include "elevators.h"
#include "../hwAPI/hardwareAPI.h"

void masterEventHandler(ElevatorInformation *elevators, pthread_t elevatorWorkDistributor)
{
	EventType e;
	EventDesc ed;

	while (1)
	{
		e = waitForEvent(&ed);

		switch (e)
		{
		case FloorButton:
			printf("Floor button pressed on floor %d.\n", ed.fbp.floor);
			break;

		case CabinButton:
			printf("Cabin button pressed in cabin %d.\n", ed.cbp.cabin);
			break;

		case Position:
			break;

		case Speed:
			printf("Speed received %f\n", ed.s.speed);
			break;

		case Error:
			printf("Error in master event handler!\n");
			break;
		}
	}
}

void floorButtonPress(int floor, FloorButtonType direction, pthread_t elevatorWorkDistributor)
{
}