#ifndef FAYT_PAIRING_HEAP_
#define FAYT_PAIRING_HEAP_
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* This is completely yoinked from CLRS */

struct pairing_heap_node {
	struct pairing_heap_node *child;
	struct pairing_heap_node *next;
	/* This points to the parent node if this is the leftmost child of the parent */
	struct pairing_heap_node *prev;
};

/*
 * Compare two pairing heap nodes
 * returning a < b will result in a min heap
 * returning a > b will result in a max heap
 * The object the node is part of can be retrieved using CONTAINER_OF
 */
typedef bool pairing_heap_cmp_func(struct pairing_heap_node *a,
								   struct pairing_heap_node *b);
struct pairing_heap {
	size_t size;
	struct pairing_heap_node *root;
	pairing_heap_cmp_func *cmp;
};

void pairing_heap_init(struct pairing_heap *heap, pairing_heap_cmp_func *cmp);

void pairing_heap_insert(struct pairing_heap *heap,
						 struct pairing_heap_node *node);
/* Get whatever is at the top of the heap and removes it */
struct pairing_heap_node *pairing_heap_pop(struct pairing_heap *heap);

/* Get whatever is at the top of the heap without removing it */
struct pairing_heap_node *pairing_heap_top(struct pairing_heap *heap);

size_t pairing_heap_size(const struct pairing_heap *heap);

#endif
