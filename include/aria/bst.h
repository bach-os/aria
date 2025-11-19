#ifndef ARIA_BST_H_
#define ARIA_BST_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BST_IS_NIL(T, N) ((N == &(T)->nil))
#define BST_NIL(T) (&(T)->nil)
#define BST_TAG_MASK 0x3 /* Low 2 bits */
#define BST_PARENT(P) \
	((struct bst_node *)((uintptr_t)((P)->parent) & ~BST_TAG_MASK))

#define BST_SET_PARENT(NODE, PARENT)                                   \
	((NODE)->parent = (void *)(((uintptr_t)(PARENT) & ~BST_TAG_MASK) | \
							   ((uintptr_t)(NODE)->parent & BST_TAG_MASK)))

struct bst_node {
	struct bst_node *left;
	struct bst_node *right;
	struct bst_node *
		parent; /* Possibly a tagged pointer, code should not rely on this pointer being valid without using BST_PARENT! */
};

typedef int (*bst_cmp_func_t)(struct bst_node *a, struct bst_node *b);

struct bst {
	struct bst_node *root;
	struct bst_node nil;
	bst_cmp_func_t cmp;
};

void bst_init(struct bst *bst, bst_cmp_func_t cmp);

struct bst_node *bst_search(struct bst *bst, struct bst_node *cmp);

struct bst_node *bst_maximum(struct bst *tree, struct bst_node *root);
struct bst_node *bst_minimum(struct bst *tree, struct bst_node *root);

struct bst_node *bst_successor(struct bst *tree, struct bst_node *elem);
struct bst_node *bst_predecessor(struct bst *tree, struct bst_node *elem);

void bst_impl_transplant(struct bst *tree, struct bst_node *u,
						 struct bst_node *v);

/* TODO: maybe add rotate_left and rotate_right instead of implementing it every time? */

#ifdef __cplusplus
}
#endif

#endif
