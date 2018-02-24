#ifndef __EVENT_QUEUE_H
#define __EVENT_QUEUE_H

#include "../hwAPI/hardwareAPI.h"

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

void init(EventQueue *q);
EventDesc front(Queue *q);
void pop(Queue *q);
void push(Queue *q, EventDesc event);

#endif