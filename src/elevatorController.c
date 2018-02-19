#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "../hwAPI/hardwareAPI.h"
#include "elevatorController.h"

static void *elevatorController(void *argument)
{
	int id = 0;
	float position;
	float velocity;

	while (1)
	{
		/* Get cabin instruction */

		/* Do things depending on it */
	}
}