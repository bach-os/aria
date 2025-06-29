#ifndef ARIA_BITMAP_H_
#define ARIA_BITMAP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <aria/alloc.h>

struct bitmap {
	unsigned char *data;
	int size;
	int resizable;

	allocator_t alloc;
};

int bitmap_alloc(struct bitmap *, int *);
int bitmap_free(struct bitmap *, int index);
int bitmap_dup(struct bitmap *, struct bitmap *);

#endif
