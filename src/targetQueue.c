#include "targetQueue.h"
#include <stdlib.h>

#include <stdio.h>

TargetQueue* target_queue_create()
{
    TargetQueue *q = (TargetQueue *) malloc(sizeof(TargetQueue));
    
    q->front = NULL;
    q->direction = Up;

    pthread_mutex_init(&q->read_mutex, NULL);
	pthread_mutex_init(&q->write_mutex, NULL);

    return q;
}

TargetQueueItem* target_queue_peek(TargetQueue *q)
{
    return q->front;
}

TargetQueueItem* target_queue_pop(TargetQueue *q)
{
    // Wait for a item to be available
	if (q->front != NULL) {
        pthread_mutex_lock(&q->read_mutex);

        TargetQueueItem *temp = q->front;
        q->front = q->front->next;

        pthread_mutex_unlock(&q->read_mutex);	
        return temp;
    }
    else
    {
        return NULL;
    }
}

int compare_target_items(TargetQueue *q, TargetQueueItem *i1,TargetQueueItem *i2)
{
    if (q->direction == Up)
        return i1->target_floor - i2->target_floor;
    else // Down
        return i2->target_floor - i1->target_floor;
}

void target_queue_push(TargetQueue *q, TargetQueueItem *item)
{
    pthread_mutex_lock(&q->write_mutex);
    
    // This is where we do the sorting and put it in the correct spot.
    TargetQueueItem *current = q->front;
    TargetQueueItem *previous = NULL;
    int cmp = 0;
    while(current != NULL)
    {
        cmp = compare_target_items(q, current, item); 
        if (cmp > 0)
        // If we found the position
        { 
            break;
        }
        else if (cmp == 0)
        // This is a duplicate item
        {
            printf("Item discarded.\n");
            target_queue_free_element(item);
            pthread_mutex_unlock(&q->write_mutex);
            return;
        }
        previous = current;
        current = current->next;
    }

    // NULL list
    if (current == NULL && previous == NULL) 
    {
        item->next = NULL;
        q->front = item;
    }
    // Fist item in list
    else if (previous == NULL) 
    {
        item->next = current;
        q->front = item;
    }
    // Insert here
    else
    {
        previous->next = item;
        item->next = current;
    }

    pthread_mutex_unlock(&q->write_mutex);	
    
}


void target_queue_free_element(TargetQueueItem *item)
{
    if(item != NULL)
        free(item);
}

// We could add some memory check here...
TargetQueueItem* target_queue_create_item(int target_floor)
{
    TargetQueueItem *item = (TargetQueueItem *) malloc(sizeof(TargetQueueItem));
    item->target_floor = target_floor;
    return item;
}
