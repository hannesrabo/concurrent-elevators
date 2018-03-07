#ifndef __ELEVATOR_CONTROLLER_H
#define __ELEVATOR_CONTROLLER_H

void *ElevatorController(void *argument);
int getProbableExtraTarget(int topFloor, int floorNumber, FloorButtonType type);

#endif