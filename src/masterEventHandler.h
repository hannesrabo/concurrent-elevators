#ifndef __MASTER_EVENT_HANDLER_H
#define __MASTER_EVENT_HANDLER_H

#include <pthread.h>

void masterEventHandler(ElevatorInformation *elevators, pthread_t elevatorWorkDistributor);

#endif