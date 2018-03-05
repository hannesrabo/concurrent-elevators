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
	sem_t *size;
	pthread_mutex_t write_mutex;
	pthread_mutex_t read_mutex;	
} EventQueue;

void event_queue_init(EventQueue *q, int unique_id);
EventQueueItem *event_queue_pop(EventQueue *q);
void event_queue_push(EventQueue *q, EventQueueItem *item);

void event_queue_free_element(EventQueueItem *item);
EventQueueItem *event_queue_create_element(EventType type, EventDesc *event);

#endif