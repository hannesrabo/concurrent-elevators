#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "eventQueue.h"

EventQueue *event_queue_create()
{
	EventQueue *q = (EventQueue *)malloc(sizeof(EventQueue));

	q->front = NULL;
	q->last = NULL;

	pthread_cond_init(&q->hasElements, NULL);
	pthread_mutex_init(&q->read_mutex, NULL);
	pthread_mutex_init(&q->write_mutex, NULL);

	return q;
}

/**
 * Blocking read/pop of the next element in the queue.
 */
EventQueueItem *event_queue_pop(EventQueue *q)
{
	// Wait for a item to be available
	pthread_mutex_lock(&q->read_mutex);

	// Wait if there is no elements in the queue.
	if (q->front == NULL)
		pthread_cond_wait(&q->hasElements, &q->read_mutex);

	EventQueueItem *temp = q->front;
	q->front = q->front->next;

	pthread_mutex_unlock(&q->read_mutex);
	return temp;
}

/**
 * Blocking but with max time.
 */
EventQueueItem *event_queue_timed_pop(EventQueue *q, long time_ms)
{
	// Wait for a item to be available
	pthread_mutex_lock(&q->read_mutex);

	// Loading time
	struct timespec ts;
	struct timeval now;
	int rt;

	gettimeofday(&now, NULL);

	ts.tv_sec = time(NULL) + time_ms / 1000;
	ts.tv_nsec = now.tv_usec * 1000 + 1000 * 1000 * (time_ms % 1000);
	ts.tv_sec += now.tv_usec / (1000 * 1000);
	ts.tv_nsec %= (1000 * 1000 * 1000);

	// Wait if there is no elements in the queue.
	if (q->front == NULL)
	{
		rt = pthread_cond_timedwait(&q->hasElements, &q->read_mutex, &ts);

		if (rt != 0)
		{
			pthread_mutex_unlock(&q->read_mutex);
			return NULL;
		}
	}

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
	// We need to aquire both locks here....
	pthread_mutex_lock(&q->read_mutex);
	pthread_mutex_lock(&q->write_mutex);

	item->next = NULL;

	// If we have an empty list
	if (q->front == NULL)
	{
		q->front = item;
		q->last = q->front; // Update end to be first element as well.
	}
	else
	{ // We can not have a null list end here. It will be the first element in that edge case
		q->last->next = item;
		q->last = q->last->next; // Update the end.
	}

	pthread_mutex_unlock(&q->write_mutex);
	pthread_mutex_unlock(&q->read_mutex);

	// Signal any waiting processes.
	pthread_cond_signal(&q->hasElements);
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
	EventQueueItem *item = (EventQueueItem *)malloc(sizeof(EventQueueItem));
	item->type = type;
	item->event = event;
	item->next = NULL;
	return item;
}