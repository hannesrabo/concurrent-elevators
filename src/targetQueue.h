#ifndef __TARGET_QUEUE_H
#define __TARGET_QUEUE_H

#include <pthread.h>
// #include "../hwAPI/hardwareAPI.h"

typedef enum {
	Up,
	Down
} Direction;

typedef struct TargetQueueItem TargetQueueItem;

struct TargetQueueItem
{
	int target_floor;
	TargetQueueItem *next;
};

typedef struct
{
	TargetQueueItem *front;
	Direction direction;
	pthread_mutex_t write_mutex;
	pthread_mutex_t read_mutex;	
} TargetQueue;

void target_queue_init(TargetQueue *q);
TargetQueueItem* target_queue_peek(TargetQueue *q);
TargetQueueItem* target_queue_pop(TargetQueue *q);
void target_queue_push(TargetQueue *q, TargetQueueItem *item);

void target_queue_free_element(TargetQueueItem *item);
TargetQueueItem* target_queue_create_item(int target_floor);

#endif