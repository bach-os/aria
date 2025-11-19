#include <aria/bst.h>
#include <stddef.h>

void bst_init(struct bst *bst, bst_cmp_func_t cmp)
{
	bst->nil.left = &bst->nil;
	bst->nil.right = &bst->nil;
	bst->nil.parent = &bst->nil;

	bst->root = &bst->nil;
	bst->cmp = cmp;
}

struct bst_node *bst_search(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *node = tree->root;

	while (!BST_IS_NIL(tree, node)) {
		int r = tree->cmp(node, elem);

		/* elem < node, move left */
		if (r < 0) {
			node = node->left;
		}

		/* elem > node, move right */
		if (r > 0) {
			node = node->right;
		}

		if (r == 0) {
			return node;
		}
	}

	return NULL;
}

struct bst_node *bst_maximum(struct bst *tree, struct bst_node *root)
{
	struct bst_node *node = root;

	while (!BST_IS_NIL(tree, node->right)) {
		node = node->right;
	}

	return node;
}

struct bst_node *bst_minimum(struct bst *tree, struct bst_node *root)
{
	struct bst_node *node = root;

	while (!BST_IS_NIL(tree, node->left)) {
		node = node->left;
	}

	return node;
}

struct bst_node *bst_successor(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *succ = elem->right;

	if (!BST_IS_NIL(tree, succ)) {
		/* Minimum of right subtree */
		return bst_minimum(tree, succ);
	}

	/* Go up and find it */
	struct bst_node *parent = BST_PARENT(elem);

	while (!BST_IS_NIL(tree, parent) && elem == parent->right) {
		elem = parent;
		parent = BST_PARENT(parent);
	}

	if (parent == tree->root) {
		return BST_NIL(tree);
	}

	return parent;
}

struct bst_node *bst_predecessor(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *succ = elem->left;

	if (!BST_IS_NIL(tree, succ)) {
		/* Maximum of left subtree */
		return bst_maximum(tree, succ);
	}

	/* Go up and find it */
	struct bst_node *parent = BST_PARENT(elem);

	while (!BST_IS_NIL(tree, parent) && elem == parent->left) {
		elem = parent;
		parent = BST_PARENT(parent);
	}

	if (parent == tree->root) {
		return BST_NIL(tree);
	}

	return parent;
}

void bst_impl_transplant(struct bst *tree, struct bst_node *u,
						 struct bst_node *v)
{
	struct bst_node *parent = BST_PARENT(u);

	if (BST_IS_NIL(tree, parent)) {
		tree->root = v;
	} else if (u == parent->left) {
		parent->left = v;
	} else if (u == parent->right) {
		parent->right = v;
	}

	BST_SET_PARENT(v, parent);
}
