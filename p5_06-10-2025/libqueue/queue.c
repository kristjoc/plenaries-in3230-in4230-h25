#include <stdlib.h>

#include "queue.h"

#include <stdio.h>

struct queue* queue_create(void)
{
	struct queue *q = (struct queue *)calloc(1, sizeof(struct queue));
	if (!q)
		return NULL;

	q->head = NULL;
	q->tail = NULL;
	q->length = 0;

	return q;
}

ssize_t queue_length(struct queue * q)
{
	if (!q)
		return -1;

	return q->length;
}

struct queue_entry * queue_entry_new(void * _data, int _id)
{
	struct queue_entry * entry = (struct queue_entry *)calloc(1,
						sizeof(struct queue_entry));
	if (!entry)
		return NULL;

	entry->data = _data;
	entry->id = _id;
	entry->next = NULL;
	entry->prev = NULL;

	return entry;
}

bool queue_is_empty(struct queue * q)
{
	if (!q) {
		perror("[ERROR]: Cannot chek the emptiness of a NULL queue");
		return true;
	}

	return q->length == 0;
}

bool queue_is_full(struct queue * q)
{
	if (!q) {
		perror("[ERROR]: Can't check the fullness of a NULL queue");
		return true;
	}

	return (queue_length(q) == MAX_QUEUE_SIZE);
}

int queue_entry_destroy(struct queue_entry * entry)
{
	if (!entry)
		return -1;

	free(entry);

	return 0;
}

int queue_head_push(struct queue * q, void * data, int id)
{
	struct queue_entry *entry;

	if (!q) {
		perror("[ERROR]: Cannot head-push on a NULL queue\n");
		return -1;
	}

	if (queue_is_full(q)) {
		printf("[WARNING]: queue is NULL OR cannot head-push on a FULL queue\n");
		return -1;
	}

	entry = queue_entry_new(data, id);
	if (!entry)
		return -1;

	/* Insert into the queue */
	if (!q->head) {
		/* If the queue was previously empty, both the head and tail must
		 * be pointed at the new entry */
		q->head = entry;
		q->tail = entry;
	} else {
		/* First entry in the list must have prev pointed back to this
		 * new entry */

		q->head->prev = entry;

		/* Only the head must be pointed at the new entry */

		q->head = entry;
	}

	q->length++;

	printf("[DEBUG]: Entry %p head-pushed into queue %p (length = %zd)\n",
							entry, q, q->length);

	return 0;
}

void * queue_head_pop(struct queue * q)
{
	struct queue_entry * entry;
	void *               data;

	if (!q) {
		perror("Cannot head-pop from a NULL queue");
		return NULL;
	}

	if (queue_is_empty(q)) {
		printf("[ERROR]: queue %p is empty, can't head-pop\n", q);
		return NULL;
	}

	/* Unlink the first entry from the head of the queue */

	entry = q->head;
	q->head = entry->next;
	data = entry->data;

	if (!q->head) {

		/* If doing this has unlinked the last entry in the queue, set
		 * tail to NULL as well. */

		q->tail = NULL;
	} else {

		/* The new first in the queue has no previous entry */

		q->head->prev = NULL;
	}

	printf("[DEBUG]: Entry %p head-popped from queue %p (length = %zd)\n", entry, q, q->length);

	queue_entry_destroy(entry);
	q->length--;

	return data;
}

int queue_tail_push(struct queue * q, void * data, int id)
{
	struct queue_entry * entry;

	if (!q) {
		perror("Cannot tail-push on a NULL queue");
		return -1;
	}

	if (queue_is_full(q) == 1) {
		printf("[WARNING]: queue is NULL | Cannot head-push on a FULL queue\n");
		return -1;
	}

	entry = queue_entry_new(data, id);
	if (!entry)
		return -1;

	/* Insert into the queue tail */

	if (!q->tail) {

		/* If the queue was previously empty, both the head and tail must
		 * be pointed at the new entry */

		q->head = entry;
		q->tail = entry;
	} else {

		/* The current entry at the tail must have next pointed to this
		 * new entry */

		q->tail->next = entry;

		/* Only the tail must be pointed at the new entry */

		q->tail = entry;
	}

	q->length++;

	printf("[DEBUG]: Entry %p tail-pushed into queue %p (length = %zd)\n",
			entry, q, q->length);

	return 0;
}

void * queue_tail_pop(struct queue * q)
{
	struct queue_entry * entry;
	void *               data;

	if (!q) {
		perror("Cannot tail-pop from a NULL queue");
		return NULL;
	}

	if (queue_is_empty(q)) {
		printf("[ERROR]: queue %p is empty, can't tail-pop\n", q);
		return NULL;
	}

	/* Unlink the first entry from the tail of the queue */

	entry = q->tail;
	q->tail = entry->prev;
	data = entry->data;

	if (!q->tail) {

		/* If doing this has unlinked the last entry in the queue, set
		 * head to NULL as well. */

		q->head = NULL;

	} else {

		/* The new entry at the tail has no next entry. */

		q->tail->next = NULL;
	}


	printf("[DEBUG]: Entry %p tail-popped from queue %p (length = %zd)\n",
							entry, q, q->length);

	queue_entry_destroy(entry);
	q->length--;

	return data;
}

void * queue_head_peek(struct queue * q)
{
	void *                data;

	if (!q) {
		perror("Cannot head-pop from a NULL queue");
		return NULL;
	}

	if (queue_is_empty(q)) {
		printf("[ERROR]: queue %p is empty, can't head-pop\n", q);
		return NULL;
	}

	data = q->head->data;

	printf("[DEBUG]: Entry %p head-peeked from queue %p (length = %zd)\n",
							q->head, q, q->length);

	return data;
}

void * queue_tail_peek(struct queue * q)
{
	void *                data;

	if (!q) {
		perror("Cannot tail-pop from a NULL queue");
		return NULL;
	}

	if (queue_is_empty(q)) {
		printf("[ERROR]: queue %p is empty, can't tail-pop\n", q);
		return NULL;
	}

	data = q->tail->data;

	printf("[DEBUG]: Entry %p tail-peeked from queue %p (length = %zd)\n",
							q->tail, q, q->length);

	return data;
}

int queue_flush(struct queue * q)
{
	/* Empty the queue */

	while (!queue_is_empty(q)) {
		queue_head_pop(q);
	}

	/* Free back the queue */

	free(q);

	printf("[DEBUG]: Queue destroyed successfully!\n");

	return 0;
}
