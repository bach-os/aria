#include "pairing_heap.h"

static struct pairing_heap_node *meld(struct pairing_heap_node *a,
									  struct pairing_heap_node *b,
									  pairing_heap_cmp_func *cmp)
{
	if (!a) {
		return b;
	}

	if (!b) {
		return a;
	}

	if (cmp(a, b)) {
		b->next = a->child;

		if (a->child) {
			a->child->prev = b;
		}

		a->child = b;
		b->prev = a;
		return a;
	}

	a->next = b->child;

	if (b->child) {
		b->child->prev = a;
	}

	b->child = a;
	a->prev = b;
	return b;
}

static struct pairing_heap_node *merge_pairs(struct pairing_heap_node *node,
											 pairing_heap_cmp_func *cmp)
{
	if (!node || !node->next) {
		return node;
	}

	struct pairing_heap_node *next = node->next;
	node->next = NULL;

	struct pairing_heap_node *merged = meld(node, next, cmp);
	while (next->next) {
		next = next->next;
		merged = meld(merged, next, cmp);
	}

	return merged;
}

void pairing_heap_init(struct pairing_heap *heap, pairing_heap_cmp_func *cmp)
{
	heap->size = 0;
	heap->root = NULL;
	heap->cmp = cmp;
}

void pairing_heap_insert(struct pairing_heap *heap,
						 struct pairing_heap_node *node)
{
	heap->root = meld(heap->root, node, heap->cmp);
	heap->size++;
}

void pairing_heap_remove(struct pairing_heap *heap,
						 struct pairing_heap_node *node)
{
	if (heap->root == node) {
		pairing_heap_pop(heap);
		return;
	}

	heap->size--;

	/* Unlink the node */
	if (node->prev->child == node) {
		node->prev->child = node->next;
	} else {
		node->prev->next = node->next;
	}

	if (node->next) {
		node->next->prev = node->prev;
	}

	if (node->child) {
		node->child->prev = NULL;
		heap->root =
			meld(heap->root, merge_pairs(node->child, heap->cmp), heap->cmp);
	}

	node->next = NULL;
	node->prev = NULL;
	node->child = NULL;
}

struct pairing_heap_node *pairing_heap_top(struct pairing_heap *heap)
{
	return heap->root;
}

struct pairing_heap_node *pairing_heap_pop(struct pairing_heap *heap)
{
	if (!heap->root) {
		return NULL;
	}

	struct pairing_heap_node *top = heap->root;

	heap->size--;

	if (!top->child) {
		heap->root = NULL;
		return top;
	}

	heap->root = merge_pairs(top->child, heap->cmp);
	top->child = NULL;

	return top;
}

size_t pairing_heap_size(const struct pairing_heap *heap)
{
	return heap->size;
}
