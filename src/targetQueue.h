#ifndef __TARGET_QUEUE_H
#define __TARGET_QUEUE_H

#include "../hwAPI/hardwareAPI.h"

typedef struct
{
	int target;
	TargetQueueItem *next;
} TargetQueueItem;

typedef struct
{
	TargetQueueItem *front;
	TargetQueueItem *last;
	unsigned int size;
	pthread_mutex_t *mutex;
} TargetQueue;

void target_queue_init(TargetQueue *q);
int target_queue_front(TargetQueue *q);
void target_queue_pop(TargetQueue *q);
void target_queue_push(TargetQueue *q, int target);

#endif