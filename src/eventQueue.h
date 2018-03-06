#ifndef __EVENT_QUEUE_H
#define __EVENT_QUEUE_H

#include <semaphore.h>
#include <pthread.h>
#include "../hwAPI/hardwareAPI.h"

typedef struct EventQueueItem EventQueueItem;

struct EventQueueItem
{
	EventDesc *event;
	EventType type;
	EventQueueItem *next;
};

typedef struct
{
	EventQueueItem *front;
	EventQueueItem *last;
	pthread_cond_t hasElements;
	pthread_mutex_t write_mutex;
	pthread_mutex_t read_mutex;
} EventQueue;

EventQueue *event_queue_create();
EventQueueItem *event_queue_pop(EventQueue *q);
void event_queue_push(EventQueue *q, EventQueueItem *item);

void event_queue_free_element(EventQueueItem *item);
EventQueueItem *event_queue_create_element(EventType type, EventDesc *event);

#endif