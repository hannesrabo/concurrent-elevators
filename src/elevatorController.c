#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../hwAPI/hardwareAPI.h"
#include "elevatorController.h"
#include "elevators.h"
#include "targetQueue.h"

#define DELTA_POSITION 0.001
#define DELTA_SPEED 0.001
#define HEARTBEAT_TIME_MS 300

void handleTargets(ElevatorStatus *status);
void updatePosition(ElevatorStatus *status, double newPosition);
void handleCabinButton(ElevatorStatus *status, int floorNumber);
void handleFloorButton(ElevatorStatus *status, int floorNumber,
					   FloorButtonType type);

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
				updatePosition(status, nextEvent->event->cp.position);
				printf("Position received: %f\n", status->position);
				break;

			case Speed:
				status->speed = nextEvent->event->s.speed;

				printf("Speed received: %f\n", status->speed);
				break;

			case CabinButton:;
				int floor = nextEvent->event->cbp.floor;
				if (floor == 32000)
				{
					printf("Cabin stop button pressed in cabin %d\n",
						   nextEvent->event->cbp.cabin);
				}
				else
				{
					// printf("Cabin button pressed in cabin %d to floor %d!\n",
					// nextEvent->event->cbp.cabin, floor);
					handleCabinButton(status, floor);
				}
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

		handleTargets(status);

		// This is where all elements are freed
		event_queue_free_element(nextEvent);
	}

	return 0;
}

void handleTargets(ElevatorStatus *status)
{
	if (status->current_movement == NotMoving)
	{
		if (status->sweep_direction ==
			SweepIdle)
		{ // Cart is not doing anything: Get a new target!
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

			// Get going now!
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
		else
		{ // Continue in the same direction as before stop.
			TargetQueueItem *item;
			if (status->sweep_direction == SweepUp)
			{
				item = target_queue_peek(status->q_up);
				if (item == NULL)
				{ // Switch direction!
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
				return;
			}

			int target_floor = item->target_floor;

			// We have arrived at the correct floor
			if (status->position == target_floor)
			{
				if (!status->door_opened)
				{ // Open the doors!
					// and Pop the element!
					if (status->sweep_direction == SweepUp)
						item = target_queue_pop(status->q_up);
					else // SweepDown
						item = target_queue_pop(status->q_down);

					target_queue_free_element(item);

					pthread_mutex_lock(&status->sendMutex);
					handleDoor(status->id, DoorOpen);
					status->door_opened = true;
					pthread_mutex_unlock(&status->sendMutex);
				}
				else
				{ // We already opened our door.
					// Let's continue the trip!
					pthread_mutex_lock(&status->sendMutex);
					handleDoor(status->id, DoorClose);

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
			else
			{ // Why would we ever stop here? (not on target floor)
			}
		}
	}
	else // Moving up or down (not idle)
	{
		TargetQueueItem *item;
		if (status->current_movement == MovingUp)
			item = target_queue_peek(status->q_up);
		else // Moving down
			item = target_queue_peek(status->q_down);

		if (item == NULL)
		{
			printf("[ERROR](%d) Moving towards unknown target\n", status->id);
			return;
		}

		double target_floor = (double)item->target_floor;

		if (fabs(target_floor - status->position) <
			DELTA_POSITION)
		{ // We have arrived at our target!
			printf("Cart %d: Stopping!\n", status->id);

			pthread_mutex_lock(&status->sendMutex);

			status->current_movement = NotMoving;
			handleMotor(status->id, MotorStop);

			status->door_opened = true;
			handleDoor(status->id, DoorOpen);

			pthread_mutex_unlock(&status->sendMutex);
		}
	}
}

void updatePosition(ElevatorStatus *status, double newPosition)
{
	status->position = newPosition;
	// This is where we need to handle when to stop the elevators.
	// printf("Position received: %f\n", status->position);
}

void handleCabinButton(ElevatorStatus *status, int floorNumber)
{
	int direction = floorNumber - status->position;
	TargetQueueItem *targetItem = target_queue_create_item(floorNumber);

	if (direction >= 0)
	{ // Elevator going up (or staying)
		target_queue_push(status->q_up, targetItem);
		printf("Cabin (%d) will visit floor %d on the way up\n", status->id,
			   floorNumber);
	}
	else
	{ // Elevator going down
		target_queue_push(status->q_down, targetItem);
		printf("Cabin (%d) will visit floor %d on the way down\n", status->id,
			   floorNumber);
	}
}

void handleFloorButton(ElevatorStatus *status, int floorNumber,
					   FloorButtonType type) {}
