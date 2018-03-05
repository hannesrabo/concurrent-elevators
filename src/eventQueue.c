#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eventQueue.h"

void event_queue_init(EventQueue *q, int unique_id)
{
	char name[50];
	sprintf(name, "/elevator_queue%d", unique_id);

	sem_unlink(name);

	q->front = NULL;
	q->last = NULL;
	q->size = sem_open(name, O_CREAT | O_EXCL, S_IWUSR, 0);

	pthread_mutex_init(&q->read_mutex, NULL);
	pthread_mutex_init(&q->write_mutex, NULL);
}

/**
 * Blocking read/pop of the next element in the queue.
 */
EventQueueItem *event_queue_pop(EventQueue *q)
{
	// Wait for a item to be available
	sem_wait(q->size);
	pthread_mutex_lock(&q->read_mutex);

	EventQueueItem *temp = q->front;
	q->front = q->front->next;

	pthread_mutex_unlock(&q->read_mutex);	
	return temp;
}

/**
 * Non blocking push of the next element
 */
void event_queue_push(EventQueue *q, EventQueueItem *item) 
{
	pthread_mutex_lock(&q->write_mutex);

	item->next = NULL;

	// If we have an empty list
	if (q->front == NULL) 
	{
		q->front = item;
		q->last = q->front; 	// Update end to be first element as well.
	}
	else
	{ // We can not have a null list end here. It will be the first element in that edge case
		q->last->next = item;
		q->last = q->last->next; // Update the end.
	}

	sem_post(q->size);
	pthread_mutex_unlock(&q->write_mutex);
}

void event_queue_free_element(EventQueueItem *item)
{
	if (item != NULL) 
	{
		free(item->event);
		free(item);
	}
}

EventQueueItem *event_queue_create_element(EventType type, EventDesc *event)
{
	EventQueueItem *item = (EventQueueItem *) malloc(sizeof(EventQueueItem));
	item->type = type;
	item->event = event;
	item->next = NULL;
	return item;
}