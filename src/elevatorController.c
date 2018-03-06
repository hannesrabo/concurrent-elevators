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
void handleFloorButton(ElevatorStatus *status, int floorNumber,
					   FloorButtonType type);

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
				printf("Speed changed: %f\n", status->speed);
				break;

			case CabinButton:;
				int floor = nextEvent->event->cbp.floor;
				handleCabinButton(status, floor);
				break;

			case FloorButton:
				printf("Cart pickup assigned at %d to cart %d\n",
					   nextEvent->event->fbp.floor, status->id);
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
			printf("Doors now closed!\n");
		}
		break;
	case DoorsOpening:
		if (get_time() - status->door_action_time > DOOR_ACTION_TIME)
		{
			status->door_status = DoorsOpen;
			printf("Doors now open!\n");
		}
		break;
	case DoorsClosed:
		// This is the normal case! Do nothing!
		break;
	case DoorsOpen:
		// Try to close the doors!
		printf("Closing doors again\n");

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

void handleTargets(ElevatorStatus *status)
{
	// Do nothing to do if we are sitting here waiting for the doors
	if (status->door_status != DoorsClosed)
		return;

	// If we are not already moving!
	if (status->current_movement == NotMoving)
	{
		// Cart is not doing anything: Get a new target!
		if (status->sweep_direction == SweepIdle)
		{
			// Extract the relevant direction
			TargetQueueItem *item;
			status->sweep_direction = SweepUp;
			item = target_queue_peek(status->q_up);
			if (item == NULL)
			{
				item = target_queue_peek(status->q_down);
				status->sweep_direction = SweepDown;
			}

			if (item == NULL)
			{ // Still no work to do
				status->sweep_direction = SweepIdle;

				return;
			}

			// If we are at the correct floor!
			if (fabs(item->target_floor - status->position) < DELTA_POSITION)
			{
				// This is not a real sweap
				status->sweep_direction = SweepIdle;
				return;
			}

			// Get going now! (activate motors in relevant direction)
			pthread_mutex_lock(&status->sendMutex);

			if (status->sweep_direction == SweepUp)
			{
				status->current_movement = MovingUp;
				handleMotor(status->id, MotorUp);
			}
			else
			{
				status->current_movement = MovingDown;
				handleMotor(status->id, MotorDown);
			}

			pthread_mutex_unlock(&status->sendMutex);
		}
		// (cart is not moving but doing a sweep) : Continue in the same direction as before stop.
		else
		{
			TargetQueueItem *item;
			if (status->sweep_direction == SweepUp)
			{
				item = target_queue_peek(status->q_up);
				if (item == NULL)
				{ // Switch direction! (we finished this direction)
					item = target_queue_peek(status->q_down);
					status->sweep_direction = SweepDown;
				}
			}
			else
			{ // SweepDown
				item = target_queue_peek(status->q_down);
				if (item == NULL)
				{ // Switch direction
					item = target_queue_peek(status->q_up);
					status->sweep_direction = SweepUp;
				}
			}

			// Edge case: No more work left!
			if (item == NULL)
			{
				status->sweep_direction = SweepIdle;

				printf("[Infor] Idling until something is in the queue!\n");
				return;
			}

			int target_floor = item->target_floor;

			// We are already at the correct floor (open doors)
			if (fabs(target_floor - status->position) < DELTA_POSITION)
			{
				printf("[Infor] Open doors on same floor!\n");

				// Remove event
				if (status->sweep_direction == SweepUp)
					item = target_queue_pop(status->q_up);
				else // SweepDown
					item = target_queue_pop(status->q_down);
				target_queue_free_element(item);

				// Doors
				pthread_mutex_lock(&status->sendMutex);
				handleDoor(status->id, DoorOpen);
				pthread_mutex_unlock(&status->sendMutex);

				status->door_action_time = get_time();
				status->door_status = DoorsOpening;
			}
			// Not on the correct floor. Time to continue sweep!
			else
			{
				printf("[infor] Continues sweep!\n");
				pthread_mutex_lock(&status->sendMutex);

				if (status->sweep_direction == SweepUp)
				{
					status->current_movement = MovingUp;
					handleMotor(status->id, MotorUp);
				}
				else
				{
					status->current_movement = MovingDown;
					handleMotor(status->id, MotorDown);
				}

				pthread_mutex_unlock(&status->sendMutex);
			}
		}
	}
	// Cart is currently moving up or down (!CartIdle)
	else
	{
		TargetQueueItem *item;
		if (status->current_movement == MovingDown)
			item = target_queue_peek(status->q_down);
		else // Moving up or not moving
			item = target_queue_peek(status->q_up);

		if (item == NULL)
		{
			printf("[ERROR](%d) Moving towards unknown target\n", status->id);
			return;
		}

		double target_floor = (double)item->target_floor;

		if (fabs(target_floor - status->position) < DELTA_POSITION)
		{ // We have arrived at our target!
			printf("Cart %d: Stopping!\n", status->id);

			status->current_movement = NotMoving;

			// Sending stop and open commands
			pthread_mutex_lock(&status->sendMutex);
			handleMotor(status->id, MotorStop);
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
			target_queue_free_element(targetItem);

		while ((targetItem = target_queue_pop(status->q_down)) != NULL)
			target_queue_free_element(targetItem);

		status->current_movement = NotMoving;
		status->sweep_direction = SweepIdle;
		pthread_mutex_lock(&status->sendMutex);
		handleMotor(status->id, MotorStop);
		pthread_mutex_unlock(&status->sendMutex);

		printf("[INFO] Panic mode in cart %d. Stopping everything!\n", status->id);

		return;
	}

	// Regular mode!
	double direction = (double)floorNumber - status->position;
	TargetQueueItem *targetItem = target_queue_create_item(floorNumber);

	if (direction >= 0)
	{ // Elevator going up (or staying)
		target_queue_push(status->q_up, targetItem);
		printf("Cabin (%d) will visit floor %d on the way up [%f]\n", status->id,
			   floorNumber, direction);
	}
	else
	{ // Elevator going down
		target_queue_push(status->q_down, targetItem);
		printf("Cabin (%d) will visit floor %d on the way down [%f]\n", status->id,
			   floorNumber, direction);
	}
}

void handleFloorButton(ElevatorStatus *status, int floorNumber,
					   FloorButtonType type) {}
