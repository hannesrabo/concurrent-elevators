#include <stdio.h>

#include "../hwAPI/hardwareAPI.h"

/* TODO: replace all fprint with real code */
void *inputHandler(void *argument)
{
	EventType e;
	EventDesc ed;

	while (1)
	{
		e = waitForEvent(&ed);

		switch (e)
		{
		case FloorButton:
			// pthread_mutex_lock(&mutex);
			fprintf(stdout, "floor button: floor %d, type %d\n", ed.fbp.floor, (int)ed.fbp.type);
			fflush(stdout);
			// pthread_mutex_unlock(&mutex);
			break;

		case CabinButton:
			// pthread_mutex_lock(&mutex);
			fprintf(stdout, "cabin button: cabin %d, floor %d\n", ed.cbp.cabin, ed.cbp.floor);
			fflush(stdout);
			// pthread_mutex_unlock(&mutex);
			break;

		case Position:
			// pthread_mutex_lock(&mutex);
			fprintf(stdout, "cabin position: cabin %d, position %f\n", ed.cp.cabin, ed.cp.position);
			fflush(stdout);
			// pthread_mutex_unlock(&mutex);
			break;

		case Speed:
			// pthread_mutex_lock(&mutex);
			fprintf(stdout, "speed: %f\n", ed.s.speed);
			fflush(stdout);
			// pthread_mutex_unlock(&mutex);
			break;

		case Error:
			// pthread_mutex_lock(&mutex);
			fprintf(stdout, "error: \"%s\"\n", ed.e.str);
			fflush(stdout);
			// pthread_mutex_unlock(&mutex);
			break;
		}
	}
}