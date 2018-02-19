/*
 * This is the main file for the project
 *
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s host-name port\n", argv[0]);
		fflush(stderr);
		exit(-1);
	}
	hostname = argv[1];
	if ((port = atoi(argv[2])) <= 0)
	{
		fprintf(stderr, "Bad port number: %s\n", argv[2]);
		fflush(stderr);
		exit(-1);
	}

	initHW(hostname, port);

	return 0;
}
