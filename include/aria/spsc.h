#ifndef ARIA_MPSC_H_
#define ARIA_MPSC_H_
#include <aria/base.h>
#include <stdatomic.h>

/**
 * Lock-free Single-Producer Single-Consumer (SPSC) Queue
 * Adapted from https://github.com/rigtorp/SPSCQueue
*/
struct spsc {
	uint32_t capacity;
	_Atomic(uint32_t) write_idx, read_idx;
	void **slots;
};

/**
 * initializes `queue` with capacity `capacity`, using the allocated buffer at `slots`
 * Note: capacity MUST be a power of 2
 */
static inline void spsc_init(struct spsc *queue, uint32_t capacity,
							 void **slots)
{
	queue->capacity = capacity;
	queue->slots = slots;
	queue->write_idx = 0;
	queue->read_idx = 0;
}

/**
 * Enqueues `item` onto `q`, blocks until there is space
 */
static inline void spsc_push(struct spsc *q, void *item)
{
	uint32_t write_idx =
		atomic_load_explicit(&q->write_idx, memory_order_relaxed);

	uint32_t read_idx =
		atomic_load_explicit(&q->read_idx, memory_order_acquire);

	uint32_t next_write_idx = (write_idx + 1) & (q->capacity - 1);

	while (next_write_idx == read_idx) {
		read_idx = atomic_load_explicit(&q->read_idx, memory_order_acquire);
	}

	q->slots[write_idx] = item;

	atomic_store_explicit(&q->write_idx, next_write_idx, memory_order_release);
}

/**
 * Pops the first item from `q`, returns NULL if empty.
 */
static inline void *spsc_pop(struct spsc *q)
{
	void *ret = NULL;

	uint32_t read_idx =
		atomic_load_explicit(&q->read_idx, memory_order_acquire);

	if (atomic_load_explicit(&q->write_idx, memory_order_acquire) == read_idx) {
		/* Empty queue */
		return NULL;
	}

	ret = q->slots[read_idx];

	uint32_t next_read_idx = (read_idx + 1) & (q->capacity - 1);

	atomic_store_explicit(&q->read_idx, next_read_idx, memory_order_release);

	return ret;
}

/**
 * Processes and removes all pending items in the queue in a single batch.
 */
static inline void spsc_pop_all(struct spsc *q, void (*handler)(void *))
{
	uint32_t head = atomic_load_explicit(&q->read_idx, memory_order_relaxed);
	uint32_t tail = atomic_load_explicit(&q->write_idx, memory_order_acquire);

	if (head == tail)
		return;

	while (head != tail) {
		void *item = q->slots[head];
		handler(item);

		head = (head + 1) & (q->capacity - 1);
	}

	atomic_store_explicit(&q->read_idx, head, memory_order_release);
}

#endif
