/*
 * A multiclient server chat application using UNIX sockets
 * File: main.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chat.h"

int main (int argc, char *argv[])
{
	int opt, sflag = 0, cflag = 0;

	while ((opt = getopt(argc, argv, "sch")) != -1) {
		switch (opt) {
		case 's':
			/* Running in server mode */
			sflag = 1;
			break;
		case 'c':
			/* Running in client mode */
			cflag = 1;
			break;
		case 'h':
		default: /* '?' */
			printf("Usage: %s \n"
			       "\t[-s] server mode\n"
			       "\t[-c] client mode\n"
			       "\t[-h] help\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (sflag)
		server();
	else if (cflag)
		client();
	else
		printf("wrong usage\n");

	return 0;
}
