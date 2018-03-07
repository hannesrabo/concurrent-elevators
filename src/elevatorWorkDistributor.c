#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
		if (best_index_cost == -1 || temp_cost < best_index_cost)
		{
			best_index_cost = temp_cost;
			best_index = current_index;
		}
	}

	return best_index;
}

/**
 * Calculate the cost when adding this item to this cart.
 */
double calculate_cart_cost(FloorButtonPressDesc *floorButtonPressDesc, ElevatorStatus *elevator)
{
	bool stops[elevator->top_floor * 2];

	for (int i = 0; i < elevator->top_floor * 2; i++)
	{
		stops[i] = false;
	}

	// Getting all stops
	TargetQueueItem *tempItem = target_queue_peek(elevator->q_up);
	while (tempItem != NULL)
	{
		stops[tempItem->target_floor] = true;

		if (tempItem->probable_extra_target != -1)
			stops[tempItem->probable_extra_target] = true;

		tempItem = tempItem->next;
	}

	tempItem = target_queue_peek(elevator->q_down);
	while (tempItem != NULL)
	{
		stops[elevator->top_floor + tempItem->target_floor] = true;

		if (tempItem->probable_extra_target != -1)
			stops[elevator->top_floor + tempItem->probable_extra_target] = true;

		tempItem = tempItem->next;
	}

	// Adding probable extra target for this item
	stops[getProbableExtraTarget(elevator->top_floor, floorButtonPressDesc->floor, floorButtonPressDesc->type)] = true;

		// This is where we are supposed to simulate the sweeep in both directions.
	// TODO: Create the algorithm.

	return -1;
}