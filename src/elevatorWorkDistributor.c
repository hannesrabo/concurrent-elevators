#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "elevatorWorkDistributor.h"
#include "elevators.h"
#include "elevatorController.h"

void handleFloorButtonPress(ElevatorWorkDistributorArgument *ewdarg, EventQueueItem *eventItem, ElevatorStatus **elevators);
int get_optimal_cart(FloorButtonPressDesc *floorButtonPressDesc, ElevatorStatus **elevators, int numberOfElevators);
double calculate_cart_cost(FloorButtonPressDesc *floorButtonPressDesc, ElevatorStatus *elevator);

/**
 * This worker thread is handeling computation of paths and work assignment
 * for the different carts.
 */
void *ElevatorWorkDistributor(void *argument)
{
	ElevatorWorkDistributorArgument *ewdarg = (ElevatorWorkDistributorArgument *)argument;
	// int numberOfElevators = ewdarg->numberOfElevators;
	ElevatorStatus **elevators = ewdarg->elevators;
	EventQueue *events = ewdarg->events;
	EventQueueItem *nextEvent;

	pthread_mutex_lock(&ewdarg->sendMutex);
	getSpeed();
	pthread_mutex_unlock(&ewdarg->sendMutex);

	while (1)
	{
		nextEvent = event_queue_pop(events);

		switch (nextEvent->type)
		{
		case FloorButton:
			// We need to push the event on the correct queue.
			handleFloorButtonPress(ewdarg, nextEvent, elevators);
			break;
		case CabinButton:
			// This is handled by the cabin itself.
			event_queue_push(elevators[nextEvent->event->cbp.cabin]->events, nextEvent);
			break;
		default:
			printf("[ERROR] Unknown event code in elevator work distributor (%d)!\n", nextEvent->type);
			exit(1);
			break;
		}
	}

	// We do not free the event here as the ownership is passed on.

	return 0;
}

/**
 * Calculates the cost for different carts and then push the event on the 
 * queue with least cost. 
 */
void handleFloorButtonPress(ElevatorWorkDistributorArgument *ewdarg, EventQueueItem *eventItem, ElevatorStatus **elevators)
{
	// This is just a test. Push it to the first cart
	int cart = get_optimal_cart(&eventItem->event->fbp, elevators, ewdarg->numberOfElevators);

	event_queue_push(elevators[cart]->events, eventItem);
}

/**
 * Calculates the cost for adding this item to all of the elevators
 */
int get_optimal_cart(FloorButtonPressDesc *floorButtonPressDesc, ElevatorStatus **elevators, int numberOfElevators)
{
	int best_index = 1;
	int current_index = 1;
	double best_index_cost = -1;

	for (current_index = 1; current_index <= numberOfElevators; current_index++)
	{
		double temp_cost = calculate_cart_cost(floorButtonPressDesc, elevators[current_index]);
		if (temp_cost == -1) // If target already exists
			return current_index;
		if (best_index_cost == -1 || temp_cost < best_index_cost)
		{
			best_index_cost = temp_cost;
			best_index = current_index;
		}
	}

	return best_index;
}

int getTargetOffset(SweepDirection elevatorDirection, SweepDirection targetDirection, int targetFloor, double elevatorPosition, int top_floor)
{
	if (elevatorDirection == targetDirection)
	{
		bool afterElevatorUp = (elevatorDirection == SweepUp && targetFloor >= elevatorPosition);
		bool afterElevatorDown = (elevatorDirection == SweepDown && targetFloor <= elevatorPosition);
		bool afterElevator = afterElevatorUp || afterElevatorDown;
		if (afterElevator)
		{
			return 0;
		}
		else
		{
			return top_floor * 2;
		}
	}
	else
	{
		return top_floor;
	}
}

// TODO!!!! Add return -1 if elevator already has in queue

/**
 * Calculate the cost when adding this item to this cart.
 */
double calculate_cart_cost(FloorButtonPressDesc *floorButtonPressDesc, ElevatorStatus *elevator)
{
	/**
	 *  We do 3 sweeps
	 * 
	 * i)   One for all floors in the same direction as elevator AND (after elevator position OR same as elevator position)
	 * ii)  One for all floors in the oposite direction
	 * iii) One for all floors in the same direction as elevator AND before current position
	 * 
	 */
	bool stops[elevator->top_floor * 3];
	int i;
	for (i = 0; i < elevator->top_floor * 3; i++)
	{
		stops[i] = false;
	}

	// Get elevator information
	SweepDirection sweep_direction = elevator->sweep_direction;

	// Figure out where to start sweeping if the elevator is idle
	if (sweep_direction == SweepIdle)
	{
		if (floorButtonPressDesc->floor > elevator->position)
		{
			sweep_direction = SweepUp;
		}
		else
		{
			sweep_direction = SweepDown;
		}
	}

	// Figure out which queue to look at
	TargetQueueItem *tempItem;
	TargetQueue *firstQueue;
	TargetQueue *secondQueue;
	if (sweep_direction == SweepUp)
	{
		firstQueue = elevator->q_up;
		secondQueue = elevator->q_down;
	}
	else
	{
		firstQueue = elevator->q_down;
		secondQueue = elevator->q_up;
	}

	tempItem = target_queue_peek(firstQueue);

	// Move past elevator for first sweep
	if (sweep_direction == SweepUp)
		while (tempItem != NULL && tempItem->target_floor <= elevator->position)
			tempItem = tempItem->next;
	else
		while (tempItem != NULL && tempItem->target_floor >= elevator->position)
			tempItem = tempItem->next;

	// Adding all stops
	while (tempItem != NULL)
	{
		// Return -1 if target already exists
		bool sameDirectionUp = (sweep_direction == SweepUp && floorButtonPressDesc->type == GoingUp);
		bool sameDirectionDown = (sweep_direction == SweepDown && floorButtonPressDesc->type == GoingDown);
		if ((sameDirectionUp || sameDirectionDown) && tempItem->target_floor == floorButtonPressDesc->floor)
			return -1;

		stops[tempItem->target_floor] = true;

		if (tempItem->probable_extra_target != -1)
			stops[tempItem->probable_extra_target] = true;

		tempItem = tempItem->next;
	}

	tempItem = target_queue_peek(secondQueue);
	while (tempItem != NULL)
	{
		// Return -1 if target already exists
		bool sameDirectionUp = (sweep_direction == SweepUp && floorButtonPressDesc->type == GoingUp);
		bool sameDirectionDown = (sweep_direction == SweepDown && floorButtonPressDesc->type == GoingDown);
		if ((!sameDirectionUp && !sameDirectionDown) && tempItem->target_floor == floorButtonPressDesc->floor)
			return -1;

		stops[elevator->top_floor + tempItem->target_floor] = true;

		if (tempItem->probable_extra_target != -1)
			stops[elevator->top_floor + tempItem->probable_extra_target] = true;

		tempItem = tempItem->next;
	}

	tempItem = target_queue_peek(firstQueue);
	while (tempItem != NULL)
	{
		// Return -1 if target already exists
		bool sameDirectionUp = (sweep_direction == SweepUp && floorButtonPressDesc->type == GoingUp);
		bool sameDirectionDown = (sweep_direction == SweepDown && floorButtonPressDesc->type == GoingDown);
		if ((sameDirectionUp || sameDirectionDown) && tempItem->target_floor == floorButtonPressDesc->floor)
			return -1;

		if (sweep_direction == SweepUp && tempItem->target_floor >= elevator->position)
			break;
		else if (sweep_direction == SweepDown && tempItem->target_floor <= elevator->position)
			break;

		stops[elevator->top_floor * 2 + tempItem->target_floor] = true;

		if (tempItem->probable_extra_target != -1)
			stops[elevator->top_floor * 2 + tempItem->probable_extra_target] = true;

		tempItem = tempItem->next;
	}

	// Adding target and probable extra target for this item
	SweepDirection targetDirection = floorButtonPressDesc->type == GoingUp ? SweepUp : SweepDown;
	int targetOffset = getTargetOffset(elevator->sweep_direction, targetDirection, floorButtonPressDesc->floor, elevator->position, elevator->top_floor);
	int probableExtraTarget = getProbableExtraTarget(elevator->top_floor, floorButtonPressDesc->floor, floorButtonPressDesc->type);
	int probableExtraTargetOffset = getTargetOffset(elevator->sweep_direction, targetDirection, probableExtraTarget, elevator->position, elevator->top_floor);
	stops[floorButtonPressDesc->floor + targetOffset] = true;
	stops[getProbableExtraTarget(elevator->top_floor, floorButtonPressDesc->floor, floorButtonPressDesc->type) + probableExtraTargetOffset] = true;

	// This is where we are supposed to simulate the sweeep in both directions.
	int floorPoint = 1;
	int doorPoint = 2;
	int points = 0;
	int lastStop = elevator->sweep_direction == SweepUp ? floor(elevator->position) : ceil(elevator->position);
	// Start at elevetors position
	for (i = lastStop; i < elevator->top_floor * 3; i++)
	{
		if (stops[i])
		{
			points += doorPoint + (i - lastStop) * floorPoint;
			lastStop = i;
		}
	}

	return points;
}