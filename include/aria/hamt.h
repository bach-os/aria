#ifndef ARIA_HAMT_H_
#define ARIA_HAMT_H_
#include <aria/alloc.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hamt_node {
	union {
		struct {
			void *value; /* Tagged pointer */
			void *key;
			struct hamt_node *next;
		} kv;

		struct {
			struct hamt_node *children;
			uint64_t bitmap;
		} branch;
	};
};

typedef bool (*hamt_cmp_func_t)(void *, void *);
typedef uint64_t (*hamt_hash_func_t)(void *);

struct hamt {
	struct hamt_node *root;
	size_t size;

	allocator_t alloc;
	hamt_cmp_func_t cmp;
	hamt_hash_func_t hash;
};

void hamt_init(struct hamt *hamt, allocator_t alloc, hamt_cmp_func_t cmp,
			   hamt_hash_func_t hash);

void hamt_insert(struct hamt *hamt, void *key, void *value);

void *hamt_get(struct hamt *hamt, void *key);

void hamt_remove(struct hamt *hamt, void *key);

void hamt_destroy(struct hamt *hamt);

#ifdef __cplusplus
}
#endif

#endif
