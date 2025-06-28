#include <aria/circular_queue.h>
#include <aria/string.h>
#include <aria/slab.h>
#include <aria/debug.h>

int circular_queue_init(struct circular_queue *queue, int data_offset,
						size_t size, size_t obj_size)
{
	if (queue == NULL)
		RETURN_ERROR;
	queue->data_offset = data_offset;
	queue->size = size;
	queue->obj_size = obj_size;
	queue->head = -1;
	queue->tail = -1;
	queue->items = 0;
	return 0;
}

int circular_queue_push(struct circular_queue *queue, const void *data)
{
	if ((queue->head == 0 && queue->tail == (queue->size - 1)) ||
		(queue->head == (queue->tail + 1))) {
		return false;
	}

	if (queue->head == -1) {
		queue->head = 0;
		queue->tail = 0;
	} else {
		if (queue->tail == (queue->size - 1)) {
			queue->tail = 0;
		} else {
			queue->tail++;
		}
	}

	memcpy((void *)queue + queue->data_offset + (queue->tail * queue->obj_size),
		   data, queue->obj_size);
	__atomic_add_fetch(&queue->items, 1, __ATOMIC_RELAXED);

	return true;
}

int circular_queue_pop(struct circular_queue *queue, void *data)
{
	if (queue->head == -1) {
		return false;
	}

	memcpy(data,
		   (void *)queue + queue->data_offset + (queue->head * queue->obj_size),
		   queue->obj_size);
	__atomic_sub_fetch(&queue->items, 1, __ATOMIC_RELAXED);

	if (queue->head == queue->tail) {
		queue->head = -1;
		queue->tail = -1;
	} else {
		if (queue->head == (queue->size - 1)) {
			queue->head = 0;
		} else {
			queue->head++;
		}
	}

	return true;
}

int circular_queue_pop_tail(struct circular_queue *queue, void *data)
{
	if (queue->head == queue->tail) {
		return false;
	}

	if (queue->tail == 0) {
		queue->tail = queue->size - 1;
	} else {
		queue->tail--;
	}

	memcpy(data,
		   (void *)queue + queue->data_offset + (queue->tail * queue->obj_size),
		   queue->obj_size);
	__atomic_sub_fetch(&queue->items, 1, __ATOMIC_RELAXED);

	return true;
}

int circular_queue_peek(struct circular_queue *queue, void *data)
{
	if (queue->head == -1) {
		return false;
	}

	memcpy(data,
		   (void *)queue + queue->data_offset + (queue->head * queue->obj_size),
		   queue->obj_size);

	return true;
}

int circular_queue_remove(struct circular_queue *queue, const void *data)
{
	if (queue == NULL || data == NULL)
		RETURN_ERROR;

	int index;
	for (index = queue->head; index < queue->tail; index++) {
		void *element = queue + queue->data_offset + (index * queue->obj_size);

		if (memcmp(element, data, queue->obj_size) == 0) {
			goto found;
		}
	}
	return -1;
found:
	for (int i = index; i < queue->tail - 1; i++) {
		void *element = queue + queue->data_offset + (i * queue->obj_size);
		memcpy(element, element + queue->obj_size, queue->obj_size);
	}

	if (queue->tail == 0) {
		queue->tail = queue->size - 1;
	} else {
		queue->tail--;
	}

	if (index == queue->head) {
		if (queue->head == queue->tail) {
			queue->head = -1;
			queue->tail = -1;
		} else {
			if (queue->head == queue->size - 1) {
				queue->head = 0;
			} else {
				queue->head++;
			}
		}
	}

	__atomic_sub_fetch(&queue->items, 1, __ATOMIC_RELAXED);

	return 0;
}

int circular_queue_flush(struct circular_queue *queue)
{
	if (queue == NULL)
		RETURN_ERROR;

	queue->head = -1;
	queue->tail = -1;
	queue->items = 0;

	return 0;
}

int circular_queue_destroy(struct circular_queue *queue)
{
	if (queue == NULL)
		RETURN_ERROR;

	free(queue);

	return 0;
}
