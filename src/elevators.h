
#ifndef __ELEVATORS_H
#define __ELEVATORS_H

#include "eventQueue.h"

typedef struct
{
    int id;
    EventQueue events;
} ElevatorInformation;

#endif