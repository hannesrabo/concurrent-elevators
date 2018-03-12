#include "targetQueue.h"
#include <stdlib.h>

#include <stdio.h>

int compare_target_items(TargetQueue *q, TargetQueueItem *i1, TargetQueueItem *i2)
{
	if (q->direction == Up)
		return i1->target_floor - i2->target_floor;
	else // Down
		return i2->target_floor - i1->target_floor;
}

TargetQueue *target_queue_create()
{
	TargetQueue *q = (TargetQueue *)malloc(sizeof(TargetQueue));

	q->front = NULL;
	q->direction = Up;

	pthread_mutex_init(&q->read_mutex, NULL);
	pthread_mutex_init(&q->write_mutex, NULL);

	return q;
}

TargetQueueItem *target_queue_peek(TargetQueue *q)
{
	return q->front;
}

TargetQueueItem *target_queue_peek_offset(TargetQueue *q, int offset)
{
	pthread_mutex_lock(&q->read_mutex);
	if (q == NULL)
		return NULL;

	TargetQueueItem *temp = q->front;
	if (q->direction == Up)
	{
		while (temp != NULL && temp->target_floor < offset)
			temp = temp->next;
	}
	// Down
	else
	{
		while (temp != NULL && temp->target_floor > offset)
			temp = temp->next;
	}

	pthread_mutex_unlock(&q->read_mutex);

	return temp;
}

TargetQueueItem *target_queue_pop(TargetQueue *q)
{
	if (q->front == NULL)
		return NULL;

	pthread_mutex_lock(&q->read_mutex);
	// pthread_mutex_lock(&q->write_mutex);

	TargetQueueItem *temp = q->front;
	q->front = q->front->next;

	// pthread_mutex_unlock(&q->write_mutex);
	pthread_mutex_unlock(&q->read_mutex);
	temp->containing_queue = NULL; // Remove item from the queue completely

	return temp;
}

TargetQueueItem *target_queue_pop_offset(TargetQueue *q, int offset)
{
	if (q->front == NULL)
		return NULL;

	pthread_mutex_lock(&q->read_mutex);
	// pthread_mutex_lock(&q->write_mutex);

	TargetQueueItem temp;
	temp.target_floor = offset;

	TargetQueueItem *current = q->front;
	TargetQueueItem *previous = NULL;
	int cmp = 0;
	while (current != NULL)
	{
		cmp = compare_target_items(q, current, &temp);
		if (cmp > 0)
		// Does not exist
		{
			// pthread_mutex_unlock(&q->write_mutex);
			pthread_mutex_unlock(&q->read_mutex);

			return NULL;
		}
		else if (cmp == 0)
		// We found the item.
		{
			previous->next = current->next;
			// pthread_mutex_unlock(&q->write_mutex);
			pthread_mutex_unlock(&q->read_mutex);

			return current;
		}
		previous = current;
		current = current->next;
	}
	// pthread_mutex_unlock(&q->write_mutex);
	pthread_mutex_unlock(&q->read_mutex);

	return NULL;
}

void target_queue_push(TargetQueue *q, TargetQueueItem *item)
{
	pthread_mutex_lock(&q->read_mutex);
	// pthread_mutex_lock(&q->write_mutex);

	// This is where we do the sorting and put it in the correct spot.
	TargetQueueItem *current = q->front;
	TargetQueueItem *previous = NULL;
	int cmp = 0;
	while (current != NULL)
	{
		cmp = compare_target_items(q, current, item);
		if (cmp > 0)
		// If we found the position
		{
			break;
		}
		else if (cmp == 0)
		// This is a duplicate item
		{
			printf("Item discarded.\n");
			free(item);
			// pthread_mutex_unlock(&q->write_mutex);
			pthread_mutex_unlock(&q->read_mutex);
			return;
		}
		previous = current;
		current = current->next;
	}

	item->containing_queue = q;

	// NULL list
	if (current == NULL && previous == NULL)
	{
		item->next = NULL;
		q->front = item;
	}
	// Fist item in list
	else if (previous == NULL)
	{
		item->next = current;
		q->front = item;
	}
	// Insert here
	else
	{
		previous->next = item;
		item->next = current;
	}

	// pthread_mutex_unlock(&q->write_mutex);
	pthread_mutex_unlock(&q->read_mutex);
}

void target_queue_free_and_remove_element(TargetQueueItem *item)
{
	if (item == NULL)
		return;

	if (item->containing_queue == NULL)
	{
		free(item);
		return;
	}

	pthread_mutex_lock(&item->containing_queue->read_mutex);
	// pthread_mutex_lock(&item->containing_queue->write_mutex);

	// remove it from the list
	TargetQueueItem *current = item->containing_queue->front;
	TargetQueueItem *previous = NULL;
	int cmp = 0;
	while (current != NULL)
	{
		cmp = compare_target_items(item->containing_queue, current, item);
		if (cmp > 0)
		// Does not exist
		{
			free(item);

			// pthread_mutex_unlock(&item->containing_queue->write_mutex);
			pthread_mutex_unlock(&item->containing_queue->read_mutex);
			return;
		}
		else if (cmp == 0)
		// We found the item.
		{
			// First item
			if (previous == NULL)
				item->containing_queue->front = current->next;
			else
				previous->next = current->next;

			free(item);

			// pthread_mutex_unlock(&item->containing_queue->write_mutex);
			pthread_mutex_unlock(&item->containing_queue->read_mutex);
			return;
		}
		previous = current;
		current = current->next;
	}

	// does not exist
	free(item);

	// pthread_mutex_unlock(&item->containing_queue->write_mutex);
	pthread_mutex_unlock(&item->containing_queue->read_mutex);
}

// TODO: We could add some memory check here...
TargetQueueItem *target_queue_create_item(int target_floor)
{
	TargetQueueItem *item = (TargetQueueItem *)malloc(sizeof(TargetQueueItem));
	item->target_floor = target_floor;
	item->probable_extra_target = -1;
	item->containing_queue = NULL;
	return item;
}

TargetQueueItem *target_queue_create_item_w_target(int target_floor, int probable_target)
{
	TargetQueueItem *item = target_queue_create_item(target_floor);
	item->probable_extra_target = probable_target;
	return item;
}

void target_queue_print_list(TargetQueue *q)
{
	TargetQueueItem *i = q->front;

	printf("[");
	while (i != NULL)
	{
		printf("%d, ", i->target_floor);
		i = i->next;
	}
	printf("]\n");
}
