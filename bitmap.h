#ifndef ARIA_BITMAP_H_
#define ARIA_BITMAP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct bitmap {
	unsigned char *data;
	int size;
	int resizable;
};

int bitmap_alloc(struct bitmap *, int *);
int bitmap_free(struct bitmap *, int index);
int bitmap_dup(struct bitmap *, struct bitmap *);

#endif
