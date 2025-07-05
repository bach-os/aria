#include <aria/pairing_heap.h>
#include <aria/debug.h>

static struct pairing_heap_node *meld(struct pairing_heap_node *a,
									  struct pairing_heap_node *b,
									  pairing_heap_cmp_func *cmp)
{
	if (cmp(a, b)) {
		if (a->child) {
			a->child->prev = b;
		}

		b->next = a->child;
		a->child = b;
		b->prev = a;
		return a;
	}

	if (b->child) {
		b->child->prev = a;
	}

	a->prev = b;
	a->next = b->child;
	b->child = a;
	return b;
}

static struct pairing_heap_node *merge_pairs(struct pairing_heap_node *node,
											 pairing_heap_cmp_func *cmp)
{
	struct pairing_heap_node *element = node;
	struct pairing_heap_node *paired = NULL;

	while (element && element->next) {
		struct pairing_heap_node *partner = element->next;
		struct pairing_heap_node *next = partner->next;

		element->prev = NULL;
		element->next = NULL;

		partner->prev = NULL;
		partner->next = NULL;

		struct pairing_heap_node *merged = meld(element, partner, cmp);
		merged->prev = paired;
		paired = merged;

		element = next;
	}

	struct pairing_heap_node *joined;

	if (element) {
		element->prev = NULL;
		joined = element;
	} else {
		struct pairing_heap_node *predec = paired->prev;
		paired->prev = NULL;

		joined = paired;
		paired = predec;
	}

	while (paired) {
		struct pairing_heap_node *predec = paired->prev;

		paired->prev = NULL;

		joined = meld(joined, paired, cmp);
		paired = predec;
	}

	return joined;
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
	node->child = NULL;
	node->next = NULL;
	node->prev = NULL;

	heap->size++;

	if (!heap->root) {
		heap->root = node;
		return;
	}
	heap->root = meld(heap->root, node, heap->cmp);
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

	struct pairing_heap_node *root = heap->root;
	struct pairing_heap_node *child = heap->root->child;

	heap->root->child = NULL;

	heap->size--;

	if (child) {
		child->prev = NULL;
		heap->root = merge_pairs(child, heap->cmp);
	} else {
		heap->root = NULL;
	}

	return root;
}

size_t pairing_heap_size(const struct pairing_heap *heap)
{
	return heap->size;
}
