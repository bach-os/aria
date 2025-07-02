#include <aria/hamt.h>
#include <aria/base.h>
#include <aria/compiler.h>

#define TAG_MASK 0x1
#define TAGGED(P) (void *)((uintptr_t)P | 1)
#define UNTAGGED(P) (void *)((uintptr_t)P & ~TAG_MASK)
#define IS_VALUE(P) (((uintptr_t)P & TAG_MASK) == 1)

#define BITS 64
#define K 6

static inline size_t popcount(uint64_t x)
{
	return __builtin_popcount(x);
}

static inline size_t get_index(uint64_t bitmap, uint32_t sparse_index)
{
	return popcount(bitmap & ((1UL << sparse_index) - 1));
}

static void extend_table(struct hamt *h, struct hamt_node *branch,
						 size_t prev_size, size_t pos)
{
	struct hamt_node *new_table =
		h->alloc.alloc(sizeof(struct hamt_node) * (prev_size + 1));

	if (prev_size) {
		/* Copy all the old elements over */
		memcpy(new_table, branch->branch.children,
			   sizeof(struct hamt_node) * pos);

		/* Shift all the elements if we're inserting in the middle */
		memcpy(&new_table[pos + 1], &branch->branch.children[pos],
			   sizeof(struct hamt_node) * (prev_size - pos));

		h->alloc.free(branch->branch.children,
					  sizeof(struct hamt_node) * prev_size);
	}

	branch->branch.children = new_table;
}

static void insert_in_branch(struct hamt *h, struct hamt_node *branch,
							 void *key, void *value, uint32_t hash_index)
{
	size_t pos, prev_size;

	prev_size = popcount(branch->branch.bitmap);

	/* Set the bit in the branch bitmap */
	branch->branch.bitmap |= (1UL << hash_index);

	pos = get_index(branch->branch.bitmap, hash_index);

	extend_table(h, branch, prev_size, pos);

	/* Insert the leaf */
	branch->branch.children[pos].kv.key = key;
	branch->branch.children[pos].kv.value = TAGGED(value);
	branch->branch.children[pos].kv.next = NULL;
}

static void convert_to_branch_and_insert(struct hamt *h,
										 struct hamt_node *old_node, void *key,
										 void *val, uint64_t hash, size_t shift)
{
	struct hamt_node *new_node, *node;
	struct hamt_node old_node_cpy = *old_node;
	size_t curr_idx, old_idx, actual_index;
	uint64_t old_hash = h->hash(old_node->kv.key);

	if (old_hash == hash) {
		/* Complete collision */
		if (!h->cmp(old_node->kv.key, key)) {
			new_node = h->alloc.alloc(sizeof(struct hamt_node));
			*new_node = *old_node;
			old_node->kv.key = key;
			old_node->kv.value = TAGGED(val);
			old_node->kv.next = new_node;
		} else {
			old_node->kv.value = TAGGED(val);
		}
		return;
	}

	curr_idx = (hash >> shift) & 0x1f;
	old_idx = (old_hash >> shift) & 0x1f;

	node = old_node;

	while (curr_idx == old_idx) {
		node->branch.bitmap = (1UL << curr_idx);
		node->branch.children = h->alloc.alloc(sizeof(struct hamt_node));

		shift += K;
		curr_idx = (hash >> shift) & 0x1f;
		old_idx = (old_hash >> shift) & 0x1f;

		node = node->branch.children;
	}

	/* Allocate space for two children (one for the new node and one for the old one) */
	node->branch.children = h->alloc.alloc(sizeof(struct hamt_node) * 2);
	node->branch.bitmap = (1UL << curr_idx) | (1UL << old_idx);

	actual_index = get_index(node->branch.bitmap, curr_idx);

	node->branch.children[get_index(node->branch.bitmap, old_idx)] =
		old_node_cpy;

	node->branch.children[actual_index].kv.value = TAGGED(val);
	node->branch.children[actual_index].kv.key = key;
	node->branch.children[actual_index].kv.next = NULL;
}

/* Descend the tree until either a leaf node is found or the node is not present */
static bool descend_tree(struct hamt_node *root, uint64_t hash, size_t *shift,
						 struct hamt_node **node)
{
	size_t idx = (hash >> *shift) & 0x1f;

	if (!(root->branch.bitmap & (1UL << idx))) {
		*node = root;
		return false;
	}

	root = &root->branch.children[get_index(root->branch.bitmap, idx)];

	while (true) {
		*node = root;

		if (IS_VALUE(root->kv.value)) {
			return true;
		}

		if (root->branch.bitmap & (1UL << idx)) {
			root = &root->branch.children[get_index(root->branch.bitmap, idx)];
			*shift += K;
			idx = (hash >> *shift) & 0x1f;
			continue;
		}

		return false;
	}
}

void hamt_insert(struct hamt *h, void *key, void *value)
{
	size_t shift = 0;
	uint64_t hash = 0;
	bool leaf_found = false;
	struct hamt_node *node;

	/* Create the root node */
	if (unlikely(!h->root)) {
		h->root = h->alloc.alloc(sizeof(struct hamt_node));
		h->root->branch.bitmap = 0;
		h->root->branch.children = NULL;
	}

	hash = h->hash(key);

	leaf_found = descend_tree(h->root, hash, &shift, &node);

	if (!leaf_found) {
		/* Insert the node in the branch */
		insert_in_branch(h, node, key, value, (hash >> shift) & 0x1f);
		return;
	}

	/* There's a collision between two leaves, create new branches until it is resolved */
	convert_to_branch_and_insert(h, node, key, value, hash, shift);
}

void *hamt_get(struct hamt *h, void *key)
{
	size_t shift = 0;
	uint64_t hash = 0;
	bool leaf_found = false;
	struct hamt_node *node = NULL;

	if (!h->root)
		return NULL;

	hash = h->hash(key);

	leaf_found = descend_tree(h->root, hash, &shift, &node);

	if (!leaf_found) {
		return NULL;
	}

	/* Possible collision */
	if (!h->cmp(key, node->kv.key)) {
		node = node->kv.next;

		while (node) {
			if (h->cmp(key, node->kv.key)) {
				return UNTAGGED(node->kv.value);
			}
			node = node->kv.next;
		}

		return NULL;
	}

	return UNTAGGED(node->kv.value);
}

void hamt_init(struct hamt *hamt, allocator_t alloc, hamt_cmp_func_t cmp,
			   hamt_hash_func_t hash)
{
	hamt->alloc = alloc;
	hamt->cmp = cmp;
	hamt->root = NULL;
	hamt->size = 0;
	hamt->hash = hash;
}

static void dealloc(struct hamt *hamt, struct hamt_node *node)
{
	if (!IS_VALUE(node->kv.value)) {
		for (size_t i = 0; i < BITS; i++) {
			if ((node->branch.bitmap & (1UL << i)) != 0) {
				dealloc(
					hamt,
					&node->branch.children[get_index(node->branch.bitmap, i)]);
			}
		}

		if (node->branch.children) {
			hamt->alloc.free(node->branch.children,
							 sizeof(struct hamt_node) *
								 popcount(node->branch.bitmap));
		}
	}
}

void hamt_destroy(struct hamt *hamt)
{
	if (hamt->root) {
		dealloc(hamt, hamt->root);
		hamt->alloc.free(hamt->root, sizeof(struct hamt_node));
	}
}

static void fold_branch(struct hamt *h, struct hamt_node *branch,
						struct hamt_node leaf)
{
	dealloc(h, branch);
	*branch = leaf;
}

static void shrink_table_to_fit(struct hamt *h, struct hamt_node *branch,
								size_t new_size, size_t prev_size, size_t pos)
{
	if (new_size == 0) {
		h->alloc.free(branch->branch.children, sizeof(struct hamt_node));
		branch->branch.children = NULL;
		return;
	}

	struct hamt_node *new_table =
		h->alloc.alloc(sizeof(struct hamt_node) * new_size);

	if (pos > 0) {
		memcpy(new_table, branch->branch.children,
			   sizeof(struct hamt_node) * pos);
	}

	if (pos < new_size) {
		memcpy(&new_table[pos], &branch->branch.children[pos + 1],
			   sizeof(struct hamt_node) * (new_size - pos));
	}

	h->alloc.free(branch->branch.children,
				  sizeof(struct hamt_node) * prev_size);
	branch->branch.children = new_table;
}

/*
 * NOTE: this code sucks because removing and keeping the HAMT in its most compact form is tedious.
 * This is also the slowest operation, but this is still fast enough and removal is uncommon.
*/
void hamt_remove(struct hamt *h, void *key)
{
	struct hamt_node *ancestors[10];
	size_t ancestors_count = 0;
	size_t shift = 0;
	uint64_t hash = h->hash(key);
	struct hamt_node *node = h->root;

	/* Descend the tree manually and keep an array of all ancestors */
	size_t idx = (hash >> shift) & 0x1f;

	/* Back out early */
	if (!(node->branch.bitmap & (1UL << idx))) {
		return;
	}

	ancestors[ancestors_count++] = node;

	node = &node->branch.children[get_index(node->branch.bitmap, idx)];

	while (true) {
		if (IS_VALUE(node->kv.value)) {
			struct hamt_node *prev = node;

			/* There's a collision, traverse the hash chain and remove the node from it */
			if (!h->cmp(node->kv.key, key)) {
				node = node->kv.next;

				while (node) {
					if (h->cmp(node->kv.key, key)) {
						prev->kv.next = node->kv.next;
						h->alloc.free(node, sizeof(struct hamt_node));
						return;
					}
					prev = node;
					node = node->kv.next;
				}

				return;
			}

			if (node->kv.next) {
				struct hamt_node *to_free = node->kv.next;
				*node = *node->kv.next;
				h->alloc.free(to_free, sizeof(struct hamt_node));
				return;
			}

			shift = shift ? shift - K : 0;
			idx = (hash >> shift) & 0x1f;

			break;
		}

		if (node->branch.bitmap & (1UL << idx)) {
			ancestors[ancestors_count++] = node;
			node = &node->branch.children[get_index(node->branch.bitmap, idx)];
			shift += K;
			idx = (hash >> shift) & 0x1f;
			continue;
		}

		/* Key not found */
		return;
	}

	/* Remove the node from hash chain */
	if (node->kv.next) {
		struct hamt_node *to_free = node->kv.next;
		*node = *node->kv.next;
		h->alloc.free(to_free, sizeof(struct hamt_node));
		return;
	}

	/* Unset bit in parent bitmap */
	struct hamt_node *parent = ancestors[ancestors_count - 1];
	size_t old_size = popcount(parent->branch.bitmap);

	/* There is only one other leaf */
	if (old_size == 2) {
		struct hamt_node other_node =
			parent->branch.children[!get_index(parent->branch.bitmap, idx)];

		if (likely(!IS_VALUE(other_node.kv.value)))
			goto shrink_table;

		size_t prune_index = ancestors_count - 1;
		size_t start = ancestors_count >= 2 ? ancestors_count - 2 :
											  ancestors_count - 1;

		/* Find the first node that can be pruned safely */
		for (size_t i = start; i > 0; i--) {
			if (popcount(ancestors[i]->branch.bitmap) > 1)
				break;

			prune_index = i;
		}

		if (unlikely(prune_index == 0)) {
			goto shrink_table;
		}

		fold_branch(h, ancestors[prune_index], other_node);

		return;
	}

shrink_table:
	parent->branch.bitmap &= ~(1UL << idx);

	shrink_table_to_fit(h, parent, old_size - 1, old_size,
						get_index(parent->branch.bitmap, idx));
}
