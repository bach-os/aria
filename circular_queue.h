#ifndef CIRCULAR_QUEUE_H_
#define CIRCULAR_QUEUE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct circular_queue {
	int data_offset;
	int size;
	int obj_size;
	int head;
	int tail;

	// This is the only atomic field. However, do not treat
	// this datastructure as a atomic. The rationale for this
	// is to allow polling for the items available in a thread
	// safe manner.
	size_t items;
};

int circular_queue_init(struct circular_queue *queue, int data_offset,
						size_t size, size_t obj_size);
int circular_queue_destroy(struct circular_queue *queue);
int circular_queue_flush(struct circular_queue *queue);
int circular_queue_push(struct circular_queue *queue, const void *data);
int circular_queue_pop(struct circular_queue *queue, void *data);
int circular_queue_pop_tail(struct circular_queue *queue, void *data);
int circular_queue_peek(struct circular_queue *queue, void *data);
int circular_queue_remove(struct circular_queue *queue, const void *data);

#endif
