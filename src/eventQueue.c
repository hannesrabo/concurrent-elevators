#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "eventQueue.h"

// TODO: We do not have to lock the queue completely before altering it.
// It is sufficient to lock front or back
void event_queue_init(EventQueue *q)
{
	q->front = NULL;
	q->last = NULL;
	q->size = 0;
	pthread_mutex_init(&q->mutex, NULL);
}

EventDesc event_queue_front(EventQueue *q)
{
	/* Do we need mutex here? */
	return q->front->event;
}

void event_queue_pop(EventQueue *q)
{
	pthread_mutex_lock(&q->mutex);
	q->size--;

	EventQueueItem *temp = q->front;
	q->front = q->front->next;
	free(temp);
	pthread_mutex_unlock(&q->mutex);
}

void event_queue_push(EventQueue *q, EventDesc event)
{
	pthread_mutex_lock(&q->mutex);
	q->size++;

	if (q->front == NULL)
	{
		q->front = (EventQueueItem *)malloc(sizeof(EventQueueItem));
		q->front->event = event;
		q->front->next = NULL;
		q->last = q->front;
	}
	else
	{
		q->last->next = (EventQueueItem *)malloc(sizeof(EventQueueItem));
		q->last->next->event = event;
		q->last->next->next = NULL;
		q->last = q->last->next;
	}
	pthread_mutex_unlock(&q->mutex);
}