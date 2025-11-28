#include <aria/rbtree.h>
#include <stdint.h>
#include <stddef.h>

/* 
 * Implementation mostly based on pseudocode from Cormen et al.
 * Refer to the book for details about the algorithms used.
 */

#define SET_COLOR(NODE, C) \
	((NODE)->parent =      \
		 (void *)((uintptr_t)(BST_PARENT(NODE)) | ((C) & BST_TAG_MASK)))

#define GRANDPARENT(N) (BST_PARENT(BST_PARENT(N)))

#define COLOR(P) ((((uintptr_t)((P)->parent)) & BST_TAG_MASK))

#define RED 1
#define BLACK 0

static void rotate_right(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *y = elem->left;
	struct bst_node *parent = BST_PARENT(elem);

	elem->left = y->right;

	if (!BST_IS_NIL(tree, y->right)) {
		BST_SET_PARENT(y->right, elem);
	}

	BST_SET_PARENT(y, parent);

	if (BST_IS_NIL(tree, parent)) {
		tree->root = y;
	} else if (elem == parent->left) {
		parent->left = y;
	} else if (elem == parent->right) {
		parent->right = y;
	}

	y->right = elem;

	BST_SET_PARENT(elem, y);
}

static void rotate_left(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *y = elem->right;
	struct bst_node *parent = BST_PARENT(elem);

	elem->right = y->left;

	if (!BST_IS_NIL(tree, y->left)) {
		BST_SET_PARENT(y->left, elem);
	}

	BST_SET_PARENT(y, parent);

	if (BST_IS_NIL(tree, parent)) {
		tree->root = y;
	} else if (elem == parent->left) {
		parent->left = y;
	} else if (elem == parent->right) {
		parent->right = y;
	}

	y->left = elem;

	BST_SET_PARENT(elem, y);
}

void rb_insert(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *node = tree->root;
	struct bst_node *parent = NULL;

	/* Initialize the new node */
	elem->parent = BST_NIL(tree);
	elem->left = BST_NIL(tree);
	elem->right = BST_NIL(tree);

	SET_COLOR(elem, RED);

	/* Tree is empty */
	if (BST_IS_NIL(tree, node)) {
		tree->root = elem;
		SET_COLOR(elem, BLACK);
		return;
	}

	/* Do a normal BST insertion */
	while (!BST_IS_NIL(tree, node)) {
		int r = tree->cmp(node, elem);

		/* elem < node, move left */
		if (r < 0) {
			/* Insert left */
			if (BST_IS_NIL(tree, node->left)) {
				node->left = elem;
				BST_SET_PARENT(elem, node);
				break;
			}

			node = node->left;
		}

		/* elem > node, move right */
		if (r > 0) {
			/* Insert right */
			if (BST_IS_NIL(tree, node->right)) {
				node->right = elem;
				BST_SET_PARENT(elem, node);
				break;
			}

			node = node->right;
		}

		/* Already inserted */
		if (r == 0) {
			return;
		}
	}

	node = elem;
	parent = BST_PARENT(node);

	/* The tree might be unbalanced, rebalance it. */
	while (COLOR(parent) == RED) {
		struct bst_node *grandparent = GRANDPARENT(node);

		/* The parent of the node is a left child */
		if (parent == grandparent->left) {
			struct bst_node *uncle = grandparent->right;

			/*
             * If the node's parent and uncle are both red, make them both black and
             * make the node's grandparent red, as a red node cannot have a red parent.
            */
			if (COLOR(uncle) == RED) {
				SET_COLOR(parent, BLACK);
				SET_COLOR(uncle, BLACK);
				SET_COLOR(grandparent, RED);

				node = grandparent;
			} else {
				if (node == parent->right) {
					node = parent;
					rotate_left(tree, node);

					parent = BST_PARENT(node);
					grandparent = GRANDPARENT(node);
				}

				SET_COLOR(parent, BLACK);
				SET_COLOR(grandparent, RED);

				rotate_right(tree, grandparent);
			}
		}

		/* This code is symmetrical to above */
		else {
			struct bst_node *uncle = grandparent->left;

			if (COLOR(uncle) == RED) {
				SET_COLOR(parent, BLACK);
				SET_COLOR(uncle, BLACK);
				SET_COLOR(grandparent, RED);

				node = grandparent;
			} else {
				if (node == parent->left) {
					node = parent;

					rotate_right(tree, node);

					parent = BST_PARENT(node);
					grandparent = GRANDPARENT(node);
				}

				SET_COLOR(parent, BLACK);
				SET_COLOR(grandparent, RED);

				rotate_left(tree, grandparent);
			}
		}

		parent = BST_PARENT(node);
	}

	SET_COLOR(tree->root, BLACK);
}

void rb_delete(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *node = elem;
	struct bst_node *child = BST_NIL(tree);

	int orig_color = COLOR(node);

	if (BST_IS_NIL(tree, node->left)) {
		child = node->right;

		/* Replace the node with its right child */
		bst_impl_transplant(tree, node, child);
	} else if (BST_IS_NIL(tree, node->right)) {
		child = node->left;

		/* Replace the node with its left child */
		bst_impl_transplant(tree, node, child);
	} else {
		/* Replace with its successor */
		node = bst_successor(tree, node);
		orig_color = COLOR(node);

		child = node->right;

		if (BST_PARENT(node) == elem) {
			BST_SET_PARENT(child, node);
		} else {
			bst_impl_transplant(tree, node, node->right);

			node->right = elem->right;

			BST_SET_PARENT(node->right, node);
		}

		bst_impl_transplant(tree, elem, node);

		node->left = elem->left;

		BST_SET_PARENT(node->left, node);
		SET_COLOR(node, COLOR(elem));
	}

	if (orig_color != BLACK) {
		/* No need to fix anything */
		return;
	}

	node = child;

	/* Fix the tree */
	while (node != tree->root && COLOR(node) == BLACK) {
		struct bst_node *parent = BST_PARENT(node);

		if (node == parent->left) {
			struct bst_node *sibling = parent->right;

			if (COLOR(sibling) == RED) {
				SET_COLOR(sibling, BLACK);
				SET_COLOR(parent, RED);

				rotate_left(tree, parent);

				parent = BST_PARENT(node);
				sibling = parent->right;
			}

			if (COLOR(sibling->left) == BLACK &&
				COLOR(sibling->right) == BLACK) {
				SET_COLOR(sibling, RED);
				node = parent;
			}

			else {
				if (COLOR(sibling->right) == BLACK) {
					SET_COLOR(sibling->left, BLACK);
					SET_COLOR(sibling, RED);

					rotate_right(tree, sibling);

					parent = BST_PARENT(node);
					sibling = parent->right;
				}

				SET_COLOR(sibling, COLOR(parent));
				SET_COLOR(parent, BLACK);
				SET_COLOR(sibling->right, BLACK);

				rotate_left(tree, parent);

				node = tree->root;
			}
		} else {
			struct bst_node *sibling = parent->left;

			if (COLOR(sibling) == RED) {
				SET_COLOR(sibling, BLACK);
				SET_COLOR(parent, RED);

				rotate_right(tree, parent);

				parent = BST_PARENT(node);
				sibling = parent->left;
			}

			if (COLOR(sibling->left) == BLACK &&
				COLOR(sibling->right) == BLACK) {
				SET_COLOR(sibling, RED);
				node = parent;
			}

			else {
				if (COLOR(sibling->left) == BLACK) {
					SET_COLOR(sibling->right, BLACK);
					SET_COLOR(sibling, RED);

					rotate_left(tree, sibling);

					parent = BST_PARENT(node);
					sibling = parent->left;
				}

				SET_COLOR(sibling, COLOR(parent));
				SET_COLOR(sibling->left, BLACK);

				SET_COLOR(parent, BLACK);

				rotate_right(tree, parent);

				node = tree->root;
			}
		}
	}

	SET_COLOR(node, BLACK);
}