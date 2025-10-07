#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

int main(int argc, char *argv[])
{
	int data[32];
	int i, *pop = NULL;

	for(i = 0; i < 32; i++)
		data[i] = i * 100;

	/* Create queue root */
	struct queue* q = queue_create();
	if (!q) {
		perror("[ERROR]: Cannot create queue");
		exit(EXIT_FAILURE);
	}

	/* head-push new elements in the queue */
	for(i = 0; i < 32; i++)
		queue_head_push(q, &data[i], i);

	/* tail-pop elements from the queue */
	for(i = 0; i < 32; i++) {
		pop = (int *)queue_tail_pop(q);
		if (pop)
			printf("tail-popped value = %d\n", *pop);
		else
			break;
	}

	/* free the allocated memory for the queue root */
	queue_flush(q);

	return 0;
}
