#ifndef __ELEVATOR_CONTROLLER_H
#define __ELEVATOR_CONTROLLER_H

typedef enum {
	Velocity,
	Position,
	addTarget,
	calculateTargetRequest
} CabinInstruction;

typedef union {
	float velocity;
	float position;
	int floor;
	FloorButtonPressDesc request;
} CabinInstructionDescription;

#endif