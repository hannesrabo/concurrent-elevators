#ifndef __TARGET_QUEUE_H
#define __TARGET_QUEUE_H

#include <pthread.h>
// #include "../hwAPI/hardwareAPI.h"

typedef enum {
	Up,
	Down
} Direction;

typedef struct TargetQueueItem TargetQueueItem;
typedef struct TargetQueue TargetQueue;

struct TargetQueueItem
{
	int target_floor;
	int probable_extra_target;
	TargetQueueItem *next;
	TargetQueue *containing_queue;
};

struct TargetQueue
{
	TargetQueueItem *front;
	Direction direction;
	pthread_mutex_t write_mutex;
	pthread_mutex_t read_mutex;
};

void target_queue_print_list(TargetQueue *q);

TargetQueue *target_queue_create();
TargetQueueItem *target_queue_peek(TargetQueue *q);
TargetQueueItem *target_queue_peek_offset(TargetQueue *q, int offset);
TargetQueueItem *target_queue_pop(TargetQueue *q);
TargetQueueItem *target_queue_pop_offset(TargetQueue *q, int offset);
void target_queue_push(TargetQueue *q, TargetQueueItem *item);

void target_queue_free_and_remove_element(TargetQueueItem *item);
TargetQueueItem *target_queue_create_item(int target_floor);
TargetQueueItem *target_queue_create_item_w_target(int target_floor, int probable_target);

#endif