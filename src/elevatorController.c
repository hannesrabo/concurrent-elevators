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
	ElevatorInformation *information = (ElevatorInformation*) argument;
	printf("I am elevator nr: %d\n", information->id);

	handleDoor(information->id, DoorOpen);

	return 0;
}