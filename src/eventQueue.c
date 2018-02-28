#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include "eventQueue.h"

// TODO: We do not have to lock the queue completely before altering it.
// It is sufficient to lock front or back
void event_queue_init(EventQueue *q, int unique_id)
{
	char name[50];
	sprintf(name, "/elevator_queue%d", unique_id);

	sem_unlink(name);

	q->front = NULL;
	q->last = NULL;
	q->size = sem_open(name, O_CREAT | O_EXCL, S_IWUSR, 0);

	pthread_mutex_init(&q->mutex, NULL);
}

EventQueueItem *event_queue_front(EventQueue *q)
{
	sem_wait(q->size);
	return q->front;
}

void event_queue_pop(EventQueue *q)
{
	pthread_mutex_lock(&q->mutex);

	if (q->front != NULL)
	{
		EventQueueItem *temp = q->front;
		q->front = q->front->next;
		free(temp->event);
		free(temp);
	}

	pthread_mutex_unlock(&q->mutex);
}

void event_queue_push(EventQueue *q, EventType type, EventDesc *event)
{
	pthread_mutex_lock(&q->mutex);

	EventDesc *heapEvent = malloc(sizeof(EventDesc));
	memcpy(heapEvent, event, sizeof(*event));

	if (q->front == NULL)
	{
		q->front = (EventQueueItem *)malloc(sizeof(EventQueueItem));
		q->front->event = heapEvent;
		q->front->type = type;
		q->front->next = NULL;
		q->last = q->front;
	}
	else
	{
		q->last->next = (EventQueueItem *)malloc(sizeof(EventQueueItem));
		q->last->next->event = heapEvent;
		q->last->next->type = type;
		q->last->next->next = NULL;
		q->last = q->last->next;
	}

	sem_post(q->size);
	pthread_mutex_unlock(&q->mutex);
}