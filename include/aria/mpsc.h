#ifndef MPSC_H_
#define MPSC_H_
#include <aria/base.h>
#include <stdatomic.h>

struct mpsc_slot {
	_Atomic(size_t) turn;
	void *item;
};

/**
 * Lock-free Multiple-Producer Single-Consumer (MPSC) queue
 * Adapted from https://github.com/rigtorp/MPMCQueue
 */
struct mpsc {
	_Atomic(size_t) head;
	size_t tail;
	size_t capacity;
	size_t shift;
	struct mpsc_slot *slots;
};

/**
 * Initializes `queue` with capacity `capacity`, using the allocated buffer at `slots`
 */
static inline void mpsc_init(struct mpsc *queue, size_t capacity,
							 struct mpsc_slot *slots)
{
	queue->capacity = capacity;
	queue->slots = slots;
	queue->head = 0;
	queue->tail = 0;
	queue->shift = log2_u32(capacity);
}

/**
 * Enqueues `item` onto `q`, blocks until there is space
 */
static inline void mpsc_push(struct mpsc *q, void *item)
{
	size_t head = atomic_fetch_add_explicit(&q->head, 1, memory_order_acq_rel);
	size_t turn = (head >> q->shift) * 2;
	struct mpsc_slot *slot = &q->slots[head & (q->capacity - 1)];

	while (atomic_load_explicit(&slot->turn, memory_order_acquire) != turn) {
#ifdef __x86_64__
		asm volatile("pause");
#endif
	}

	slot->item = item;

	atomic_store_explicit(&slot->turn, turn + 1, memory_order_release);
}

/**
 * Tries popping the last item from `q`.
 * Returns 0 on success, 1 if the operation must be retried and -1 if the list is empty
 */
static inline int mpsc_try_pop(struct mpsc *q, void **out)
{
	size_t tail = q->tail;
	struct mpsc_slot *slot = &q->slots[tail & (q->capacity - 1)];
	size_t turn = (tail >> q->shift) * 2;

	if (atomic_load_explicit(&slot->turn, memory_order_acquire) != turn + 1) {
		if (atomic_load_explicit(&q->head, memory_order_acquire) == q->tail) {
			/* Empty */
			return -1;
		}

		/* Writing in progress */
		return 1;
	}

	tail++;

	*out = slot->item;

	slot->item = NULL;

	atomic_store_explicit(&slot->turn, turn + 2, memory_order_release);
	return 0;
}

/**
 * Pops the last item from `q`, returns NULL if empty.
 */
static inline void *mpsc_pop(struct mpsc *q)
{
	void *ret;
	int res = 0;

	do {
		res = mpsc_try_pop(q, &ret);

		if (res == -1) {
			return NULL;
		}

	} while (res == 1);

	return ret;
}

#endif