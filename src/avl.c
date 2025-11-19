#include <aria/avl.h>
#include <stddef.h>
#include <stdint.h>
#include <aria/base.h>

#define BF(P) ((uintptr_t)((P)->parent) & BST_TAG_MASK)

#define SET_BF(NODE, BF) \
	((NODE)->parent =    \
		 (void *)((uintptr_t)(BST_PARENT(NODE)) | ((BF) & BST_TAG_MASK)))

#define AVL_MINUS_ONE 1 /* Left-heavy */
#define AVL_ZERO 0 /* Balanced */
#define AVL_ONE 2 /* Right-heavy */

static struct bst_node *rotate_left(struct bst *tree, struct bst_node *x,
									struct bst_node *z)
{
	x->right = z->left;

	if (!BST_IS_NIL(tree, z->left)) {
		BST_SET_PARENT(z->left, x);
	}

	z->left = x;

	BST_SET_PARENT(x, z);

	if (BF(z) == AVL_ZERO) {
		SET_BF(x, AVL_ONE);
		SET_BF(z, AVL_MINUS_ONE);
	} else {
		SET_BF(x, AVL_ZERO);
		SET_BF(z, AVL_ZERO);
	}

	return z;
}

static struct bst_node *rotate_right(struct bst *tree, struct bst_node *x,
									 struct bst_node *z)
{
	x->left = z->right;

	if (!BST_IS_NIL(tree, z->right)) {
		BST_SET_PARENT(z->right, x);
	}

	z->right = x;

	BST_SET_PARENT(x, z);

	if (BF(z) == AVL_ZERO) {
		SET_BF(x, AVL_MINUS_ONE);
		SET_BF(z, AVL_ONE);
	} else {
		SET_BF(x, AVL_ZERO);
		SET_BF(z, AVL_ZERO);
	}

	return z;
}

static struct bst_node *rotate_right_left(struct bst *tree, struct bst_node *x,
										  struct bst_node *z)
{
	struct bst_node *y = z->left;
	struct bst_node *t2 = y->right;

	/* first, rotate right at Z */
	z->left = t2;
	if (!BST_IS_NIL(tree, t2)) {
		BST_SET_PARENT(t2, z);
	}

	y->right = z;

	BST_SET_PARENT(z, y);

	struct bst_node *t1 = y->left;

	/* second, rotate left at X */
	x->right = t1;

	if (!BST_IS_NIL(tree, t1)) {
		BST_SET_PARENT(t1, x);
	}

	y->left = x;
	BST_SET_PARENT(x, y);

	if (BF(y) == AVL_ZERO) {
		SET_BF(x, AVL_ZERO);
		SET_BF(z, AVL_ZERO);
	} else if (BF(y) == AVL_ONE) {
		SET_BF(x, AVL_MINUS_ONE);
		SET_BF(z, AVL_ZERO);

	} else {
		SET_BF(x, AVL_ZERO);
		SET_BF(z, AVL_ONE);
	}

	SET_BF(y, AVL_ZERO);

	return y;
}

static struct bst_node *rotate_left_right(struct bst *tree, struct bst_node *x,
										  struct bst_node *z)
{
	struct bst_node *y = z->right;
	struct bst_node *t2 = y->left;

	/* first, rotate left at Z */
	z->right = t2;
	if (!BST_IS_NIL(tree, t2)) {
		BST_SET_PARENT(t2, z);
	}

	y->left = z;
	BST_SET_PARENT(z, y);

	struct bst_node *t3 = y->right;

	/* second, rotate right at X */
	x->left = t3;

	if (!BST_IS_NIL(tree, t3)) {
		BST_SET_PARENT(t3, x);
	}

	y->right = x;
	BST_SET_PARENT(x, y);

	if (BF(y) == AVL_ZERO) {
		SET_BF(x, AVL_ZERO);
		SET_BF(z, AVL_ZERO);
	} else if (BF(y) == AVL_MINUS_ONE) {
		SET_BF(x, AVL_ONE);
		SET_BF(z, AVL_ZERO);

	} else {
		SET_BF(x, AVL_ZERO);
		SET_BF(z, AVL_MINUS_ONE);
	}

	SET_BF(y, AVL_ZERO);

	return y;
}

void avl_insert(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *node = tree->root;
	struct bst_node *parent = &tree->nil;
	struct bst_node *original_parent = BST_NIL(tree);
	struct bst_node *n = BST_NIL(tree);

	/* Initialize the new node */
	elem->parent = BST_NIL(tree);
	elem->left = BST_NIL(tree);
	elem->right = BST_NIL(tree);

	SET_BF(elem, 0);

	/* Tree is empty */
	if (BST_IS_NIL(tree, node)) {
		tree->root = elem;
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

	/* Now the tree may be unbalanced, rebalance it */
	for (parent = BST_PARENT(node); parent != BST_NIL(tree);
		 parent = BST_PARENT(node)) {
		original_parent = BST_PARENT(parent);

		if (node == parent->right) {
			if (BF(parent) == AVL_ONE) {
				/*
			     * This node has been inserted on the right of the parent, and the parent already has a balance factor of 1 (right-heavy),
				 * the parent's balance factor would then become +2, which breaks the invariant.
				 * We need to rebalance.
				 */
				if (BF(node) == AVL_MINUS_ONE) {
					n = rotate_right_left(tree, parent, node);
				} else {
					/*
					 * This is a case where the tree looks like this:
					 *  1
					 *   \
					 *    2
					 *    \
					 *     3
					 * We need to rotate the subtree so it looks like:
					 *      2
					 *     / \
					 *    1  3
					 * In the code, `parent` would be 1 and `node` would be 2
					 */
					n = rotate_left(tree, parent, node);
				}
			} else {
				/*
				 * The balance factor is either 0 or -1, increase it.
				 * If it was -1, now it is 0 so the subtree is perfectly balanced, we can stop.
				 * If it was 0, now it is +1, so we may need to check higher up for imbalances.
				 */
				if (BF(parent) == AVL_MINUS_ONE) {
					SET_BF(parent, AVL_ZERO);
					break;
				}

				SET_BF(parent, AVL_ONE);
				node = parent;
				continue;
			}
		}

		/*
		 * This node is the left child.
		 * The logic is the same as the other case but inverted. 
		 */
		else {
			if (BF(parent) == AVL_MINUS_ONE) {
				/*
			         * This node has been inserted on the left of the parent, and the parent already has a balance factor of -1 (left-heavy),
				 * the parent's balance factor would then become -2, which breaks the invariant.
				 * We need to rebalance.
				 */
				if (BF(node) == AVL_ONE) {
					n = rotate_left_right(tree, parent, node);
				} else {
					n = rotate_right(tree, parent, node);
				}
			} else {
				if (BF(parent) == AVL_ONE) {
					SET_BF(parent, AVL_ZERO);
					break;
				}
				SET_BF(parent, AVL_MINUS_ONE);
				node = parent;
				continue;
			}
		}

		/* Adopt the new subtree root */
		BST_SET_PARENT(n, original_parent);

		if (!BST_IS_NIL(tree, original_parent)) {
			if (parent == original_parent->left) {
				original_parent->left = n;
			} else {
				original_parent->right = n;
			}
		} else {
			tree->root = n;
		}

		break;
	}
}

void avl_delete(struct bst *tree, struct bst_node *elem)
{
	struct bst_node *node = BST_NIL(tree);
	struct bst_node *original_parent = BST_NIL(tree);
	struct bst_node *parent = BST_NIL(tree);
	bool was_on_left = false;
	int b;

	node = elem;
	parent = BST_PARENT(node);

	if (!BST_IS_NIL(tree, parent) && parent->left == node) {
		was_on_left = true;
	}

	/* Standard BST deletion: */
	/* Element has one child: */
	if (BST_IS_NIL(tree, elem->left)) {
		bst_impl_transplant(tree, elem, elem->right);
	} else if (BST_IS_NIL(tree, elem->right)) {
		bst_impl_transplant(tree, elem, elem->left);
	} else if (!BST_IS_NIL(tree, elem->right) &&
			   !BST_IS_NIL(tree, elem->left)) {
		/* Element has two children, get its in-order successor */
		struct bst_node *succ = bst_successor(tree, node);

		parent = BST_PARENT(succ);

		was_on_left =
			(!BST_IS_NIL(tree, parent) ? succ == parent->left : false);

		if (parent != elem) {
			bst_impl_transplant(tree, succ, succ->right);
			succ->right = elem->right;
			BST_SET_PARENT(succ->right, succ);
		} else {
			/* The successor is the direct child, so start from succ itself */
			parent = succ;
		}

		bst_impl_transplant(tree, elem, succ);

		/* Replace `elem` with `succ`, essentially */
		succ->left = elem->left;
		BST_SET_PARENT(succ->left, succ);
		SET_BF(succ, BF(elem));

		node = succ;
	}

	/* Now the tree may be unbalanced, rebalance it */
	for (; parent != BST_NIL(tree); parent = original_parent) {
		original_parent = BST_PARENT(parent);

		if (node == parent->left || was_on_left) {
			was_on_left = false;

			if (BF(parent) == AVL_ONE) {
				struct bst_node *sibling = parent->right;
				b = BF(sibling);

				if (b == AVL_MINUS_ONE) {
					node = rotate_right_left(tree, parent, sibling);
				} else {
					node = rotate_left(tree, parent, sibling);
				}
			} else {
				if (BF(parent) == AVL_ZERO) {
					SET_BF(parent, AVL_ONE);
					break;
				}
				node = parent;
				SET_BF(parent, AVL_ZERO);
				continue;
			}
		} else {
			if (BF(parent) == AVL_MINUS_ONE) {
				struct bst_node *sibling = parent->left;
				b = BF(sibling);

				if (b == AVL_ONE) {
					node = rotate_left_right(tree, parent, sibling);
				} else {
					node = rotate_right(tree, parent, sibling);
				}
			} else {
				if (BF(parent) == AVL_ZERO) {
					SET_BF(parent, AVL_MINUS_ONE);
					break;
				}
				node = parent;
				SET_BF(node, AVL_ZERO);
				continue;
			}
		}

		/* Adopt the new subtree root */
		BST_SET_PARENT(node, original_parent);

		if (!BST_IS_NIL(tree, original_parent)) {
			if (parent == original_parent->left) {
				original_parent->left = node;
			} else {
				original_parent->right = node;
			}
		} else {
			tree->root = node;
		}

		if (b == AVL_ZERO) {
			break;
		}
	}
}
