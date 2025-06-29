#ifndef ARIA_LOCK_H_
#define ARIA_LOCK_H_

#include <stdbool.h>

struct spinlock {
	char lock;
	int interrupts;
};

#define SPINLOCK_INITIALIZER { .lock = 0, .interrupts = 0 }

static inline void raw_spinlock(void *lock)
{
	while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE))
		;
}

static inline void raw_spinrelease(void *lock)
{
	__atomic_clear(lock, __ATOMIC_RELEASE);
}

static inline void spinlock(struct spinlock *spinlock)
{
	raw_spinlock(&spinlock->lock);
}

static inline void spinrelease(struct spinlock *spinlock)
{
	raw_spinrelease(&spinlock->lock);
}

#endif
