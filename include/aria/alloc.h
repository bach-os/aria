#ifndef ARIA_ALLOC_H_
#define ARIA_ALLOC_H_
#include <stddef.h>
#include <stdint.h>

typedef struct allocator {
	void *(*alloc)(size_t);
	void (*free)(void *, size_t);
	void *(*realloc)(void *, size_t, size_t);
} allocator_t;

#endif
