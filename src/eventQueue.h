#ifndef __EVENT_QUEUE_H
#define __EVENT_QUEUE_H

#include "../hwAPI/hardwareAPI.h"

typedef struct
{
	EventDesc event;
	EventQueueItem *next;
} EventQueueItem;

typedef struct
{
	EventQueueItem *front;
	EventQueueItem *last;
	unsigned int size;
	pthread_mutex_t *mutex;
} EventQueue;

void event_queue_init(EventQueue *q);
EventDesc event_queue_front(EventQueue *q);
void event_queue_pop(EventQueue *q);
void event_queue_push(EventQueue *q, EventDesc event);

#endif