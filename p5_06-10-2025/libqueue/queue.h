#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <sys/types.h>

#define MAX_QUEUE_SIZE 16

struct queue_entry {
	struct queue_entry *next;
	struct queue_entry *prev;
	int id;
	void *           data;
};

struct queue {
	struct queue_entry *head;
	struct queue_entry *tail;
	size_t           length;
};

struct  queue* queue_create(void);
ssize_t queue_length(struct queue * q);
bool    queue_is_empty(struct queue * q);
bool    queue_is_full(struct queue * q);
struct  queue_entry * queue_entry_new(void * data, int id);
int     queue_entry_destroy(struct queue_entry * entry);
int     queue_head_push(struct queue * q, void * data, int id);
void *  queue_head_pop(struct queue * q);
void *  queue_head_peek(struct queue * q);
int     queue_tail_push(struct queue * p, void * data, int id);
void *  queue_tail_pop(struct queue * q);
void *  queue_tail_peek(struct queue * q);
int     queue_flush(struct queue * q);

#endif
