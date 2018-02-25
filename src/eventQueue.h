#ifndef __EVENT_QUEUE_H
#define __EVENT_QUEUE_H

#include <pthread.h>
#include <semaphore.h>

#include "../hwAPI/hardwareAPI.h"

typedef struct EventQueueItem EventQueueItem;

struct EventQueueItem
{
	EventDesc event;
	EventQueueItem *next;
};

typedef struct
{
	EventQueueItem *front;
	EventQueueItem *last;
	sem_t *size;
	pthread_mutex_t mutex;
} EventQueue;

void event_queue_init(EventQueue *q, int unique_id);
EventDesc event_queue_front(EventQueue *q);
void event_queue_pop(EventQueue *q);
void event_queue_push(EventQueue *q, EventDesc event);

#endif