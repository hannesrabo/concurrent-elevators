#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "../hwAPI/hardwareAPI.h"
#include "elevatorController.h"
#include "elevators.h"
#include "targetQueue.h"

#define DELTA_POSITION 0.05
#define DELTA_SPEED 0.001
#define HEARTBEAT_TIME_MS 300
#define DOOR_ACTION_TIME 1

void handleTargets(ElevatorStatus *status);
void handleDoorStatus(ElevatorStatus *status);
void handleCabinButton(ElevatorStatus *status, int floorNumber);
void handleFloorButton(ElevatorStatus *status, int floorNumber, FloorButtonType type);

double get_time()
{
	/* timer */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) + 1.0e-6 * (tv.tv_usec);
}

void *ElevatorController(void *elevator_status_arg)
{
	ElevatorStatus *status = (ElevatorStatus *)elevator_status_arg;
	EventQueueItem *nextEvent;

	// Handle events
	while (1)
	{
		nextEvent = event_queue_timed_pop(status->events, HEARTBEAT_TIME_MS);

		if (nextEvent != NULL)
			switch (nextEvent->type)
			{
			case Position:
				status->position = nextEvent->event->cp.position;
				break;

			case Speed:
				status->speed = nextEvent->event->s.speed;
				printf("[INFO](%d) Speed changed: %f\n", status->id, status->speed);
				break;

			case CabinButton:;
				int floor = nextEvent->event->cbp.floor;
				handleCabinButton(status, floor);
				break;

			case FloorButton:
				handleFloorButton(status, nextEvent->event->fbp.floor, nextEvent->event->fbp.type);
				break;

			default:
				printf("[ERROR] Elevator controler %d got invalid event code (%d)!",
					   status->id, nextEvent->type);
				exit(1);
				break;
			}

		// Do the heartbeat control updates
		handleTargets(status);
		handleDoorStatus(status);

		// This is where all elements are freed
		event_queue_free_element(nextEvent);
	}

	return 0;
}

void handleDoorStatus(ElevatorStatus *status)
{
	switch (status->door_status)
	{
	case DoorsClosing:
		if (get_time() - status->door_action_time > DOOR_ACTION_TIME)
		{
			status->door_status = DoorsClosed;
			printf("[INFO](%d) Doors closed!\n", status->id);
		}
		break;
	case DoorsOpening:
		if (get_time() - status->door_action_time > DOOR_ACTION_TIME)
		{
			status->door_status = DoorsOpen;
			printf("[INFO](%d) Doors open!\n", status->id);
		}
		break;
	case DoorsClosed:
		// This is the normal case! Do nothing!
		break;
	case DoorsOpen:
		// Try to close the doors!

		status->door_status = DoorsClosing;
		status->door_action_time = get_time();

		pthread_mutex_lock(&status->sendMutex);
		handleDoor(status->id, DoorClose);
		pthread_mutex_unlock(&status->sendMutex);

		break;
	default:
		printf("[ERROR] (%d) Invalid door position! \n", status->id);
		break;
	}
}

TargetQueueItem *get_target_item(ElevatorStatus *status)
{
	TargetQueueItem *item;
	if (status->sweep_direction == SweepIdle)
		status->sweep_direction = SweepUp;

	if (status->sweep_direction == SweepUp)
	{
		item = target_queue_peek_offset(status->q_up, (int)status->position);
		if (item == NULL)
		{ // Switch direction! (we finished this direction)
			item = target_queue_peek(status->q_down);
			status->sweep_direction = SweepDown;
		}
	}
	else
	{ // SweepDown
		item = target_queue_peek_offset(status->q_down, (int)status->position);
		if (item == NULL)
		{ // Switch direction
			item = target_queue_peek(status->q_up);
			status->sweep_direction = SweepUp;
		}
	}

	if (item == NULL)
		status->sweep_direction = SweepIdle;

	return item;
}

void handleTargets(ElevatorStatus *status)
{
	// Do nothing to do if we are sitting here waiting for the doors
	if (status->door_status != DoorsClosed)
		return;

	// Extract the relevant direction
	TargetQueueItem *item = get_target_item(status);

	// Cart is not doing anything: Get a new target!
	if (status->sweep_direction == SweepIdle)
	{
		// No work
		if (item == NULL)
			return;

		// If we are at the correct floor!
		if (fabs(item->target_floor - status->position) < DELTA_POSITION)
		{
			// This is not a real sweap
			status->sweep_direction = SweepIdle;
			return;
		}

		// Get going now! (activate motors in relevant direction)
		pthread_mutex_lock(&status->sendMutex);

		if (item->target_floor > status->position)
		{
			status->sweep_direction = SweepUp;
			handleMotor(status->id, MotorUp);
		}
		else
		{
			status->sweep_direction = SweepDown;
			handleMotor(status->id, MotorDown);
		}

		pthread_mutex_unlock(&status->sendMutex);
	}
	// (cart is doing a sweep) : Continue in the same direction as before stop.
	else
	{
		// Start ideling
		if (item == NULL)
		{
			status->sweep_direction = SweepIdle;
			printf("[INFO](%d) Now idle!\n", status->id);
			return;
		}

		int target_floor = item->target_floor;

		// We are already at the correct floor (open doors)
		if (fabs(target_floor - status->position) < DELTA_POSITION)
		{

			printf("[INFO](%d) Stop and open doors!\n", status->id);

			// Remove event
			target_queue_free_and_remove_element(item);

			// Sending stop and open commands
			pthread_mutex_lock(&status->sendMutex);
			handleMotor(status->id, MotorStop);
			handleDoor(status->id, DoorOpen);
			pthread_mutex_unlock(&status->sendMutex);

			status->door_action_time = get_time();
			status->door_status = DoorsOpening;
		}
		// Not on the correct floor. Time to continue sweep!
		else
		{
			// printf("[INFO](%d) Continues sweep!\n", status->id);
			pthread_mutex_lock(&status->sendMutex);

			if (item->target_floor > status->position)
			{
				status->sweep_direction = SweepUp;
				handleMotor(status->id, MotorUp);
			}
			else
			{
				status->sweep_direction = SweepDown;
				handleMotor(status->id, MotorDown);
			}

			pthread_mutex_unlock(&status->sendMutex);
		}
	}
}

void handleCabinButton(ElevatorStatus *status, int floorNumber)
{
	// Panik mode!!
	if (floorNumber == 32000)
	{
		TargetQueueItem *targetItem;
		while ((targetItem = target_queue_pop(status->q_up)) != NULL)
			free(targetItem);

		while ((targetItem = target_queue_pop(status->q_down)) != NULL)
			free(targetItem);

		status->sweep_direction = SweepIdle;
		pthread_mutex_lock(&status->sendMutex);
		handleMotor(status->id, MotorStop);
		pthread_mutex_unlock(&status->sendMutex);

		printf("[INFO](%d) Panic mode. Stopping everything!\n", status->id);

		return;
	}

	// Regular mode!
	double direction = (double)floorNumber - status->position;
	TargetQueueItem *targetItem = target_queue_create_item(floorNumber);

	if (direction >= 0)
	{ // Elevator going up (or staying)
		target_queue_push(status->q_up, targetItem);
		printf("[INFO](%d) Cabin will visit floor %d on the way up. (internal)\n", status->id, floorNumber);
	}
	else
	{ // Elevator going down
		target_queue_push(status->q_down, targetItem);
		printf("[INFO](%d) Cabin will visit floor %d on the way down. (internal)\n", status->id, floorNumber);
	}
}

/**
 * This function calculates a probable extra target for external button presses.
 */
int getProbableExtraTarget(int topFloor, int floorNumber, FloorButtonType type)
{
	int probable_extra_target;

	if (type == GoingUp)
	{
		probable_extra_target = (int)((topFloor + floorNumber) / 2);
		if (probable_extra_target == floorNumber)
			probable_extra_target++;
	}
	else // GoingDown
	{
		probable_extra_target = floorNumber / 2;
		if (probable_extra_target == floorNumber)
			probable_extra_target--;
	}

	return probable_extra_target;
}

/**
 * This is where we handle events from passengers outside of the cart.
 */
void handleFloorButton(ElevatorStatus *status, int floorNumber, FloorButtonType type)
{
	// This menas that we must add it to the correct list depending on
	// intended travel direction.
	TargetQueue *tempQueue;
	int probable_extra_target = getProbableExtraTarget(status->top_floor, floorNumber, type);

	if (type == GoingUp)
	{
		tempQueue = status->q_up;
		printf("[INFO](%d) Cabin will visit floor %d on the way up. Extra: %d\n", status->id, floorNumber, probable_extra_target);
	}
	else // GoingDown
	{
		tempQueue = status->q_down;
		printf("[INFO](%d) Cabin will visit floor %d on the way down. Extra: %d\n", status->id, floorNumber, probable_extra_target);
	}

	TargetQueueItem *targetItem = target_queue_create_item_w_target(floorNumber, probable_extra_target);
	target_queue_push(tempQueue, targetItem);
}
