#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct
{
	EventDesc event;
	QueueItem *next;
} QueueItem;

typedef struct
{
	QueueItem *front;
	QueueItem *last;
	unsigned int size;
	pthread_mutex_t *mutex;
} EventQueue;

void init(EventQueue *q)
{
	q->front = NULL;
	q->last = NULL;
	q->size = 0;
	pthread_mutex_init(q->mutex);
}

EventDesc front(Queue *q)
{
	/* Do we need mutex here? */
	return q->front->event;
}

void pop(Queue *q)
{
	pthread_mutex_lock(q->mutex);
	q->size--;

	QueueItem *temp = q->front;
	q->front = q->front->next;
	free(temp);
	pthread_mutex_unlock(q->mutex);
}

void push(Queue *q, EventDesc event)
{
	pthread_mutex_lock(q->mutex);
	q->size++;

	if (q->front == NULL)
	{
		q->front = (QueueItem *)malloc(sizeof(QueueItem));
		q->front->data = data;
		q->front->next = NULL;
		q->last = q->front;
	}
	else
	{
		q->last->next = (QueueItem *)malloc(sizeof(QueueItem));
		q->last->next->data = data;
		q->last->next->next = NULL;
		q->last = q->last->next;
	}
	pthread_mutex_unlock(q->mutex);
}