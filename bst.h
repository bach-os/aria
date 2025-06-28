#ifndef ARIA_BST_H_
#define ARIA_BST_H_

#define BST_GENERIC_INSERT(TABLE_ROOT, BASE, NODE)    \
	({                                                \
		int ret = 0;                                  \
		if ((NODE) == NULL) {                         \
			ret = -1;                                 \
		} else {                                      \
			(NODE)->left = NULL;                      \
			(NODE)->right = NULL;                     \
			__typeof__(TABLE_ROOT) root = TABLE_ROOT; \
			__typeof__(TABLE_ROOT) parent = NULL;     \
			for (; root;) {                           \
				parent = root;                        \
				if (root->BASE > (NODE)->BASE)        \
					root = root->left;                \
				else                                  \
					root = root->right;               \
			}                                         \
			(NODE)->parent = parent;                  \
			if (parent == NULL)                       \
				TABLE_ROOT = (NODE);                  \
			else if (parent->BASE > (NODE)->BASE)     \
				parent->left = (NODE);                \
			else                                      \
				parent->right = (NODE);               \
		}                                             \
		ret;                                          \
	})

#define BST_GENERIC_DELETE(TABLE_ROOT, NODE)                          \
	({                                                                \
		int ret = 0;                                                  \
		if ((NODE) == NULL) {                                         \
			ret = -1;                                                 \
		} else {                                                      \
			__typeof__(NODE) parent = (NODE)->parent;                 \
			if ((NODE)->left == NULL && (NODE)->right == NULL) {      \
				if (parent == NULL)                                   \
					TABLE_ROOT = NULL;                                \
				else if (parent->left == (NODE))                      \
					parent->left = NULL;                              \
				else                                                  \
					parent->right = NULL;                             \
			} else if ((NODE)->left && (NODE)->right == NULL) {       \
				if (parent == NULL)                                   \
					TABLE_ROOT = (NODE)->left;                        \
				else if (parent->left == (NODE))                      \
					parent->left = (NODE)->left;                      \
				else                                                  \
					parent->right = (NODE)->left;                     \
				(NODE)->left->parent = parent;                        \
			} else if ((NODE)->right && (NODE)->left == NULL) {       \
				if (parent == NULL)                                   \
					TABLE_ROOT = (NODE)->right;                       \
				else if (parent->left == (NODE))                      \
					parent->left = (NODE)->right;                     \
				else                                                  \
					parent->right = (NODE)->right;                    \
				(NODE)->right->parent = parent;                       \
			} else {                                                  \
				__typeof__(NODE) successor = (NODE)->right;           \
				for (; successor->left;)                              \
					successor = successor->left;                      \
				if (successor->parent != (NODE)) {                    \
					successor->parent->left = successor->right;       \
					if (successor->right)                             \
						successor->right->parent = successor->parent; \
					successor->right = (NODE)->right;                 \
					if (successor->right)                             \
						successor->right->parent = successor;         \
				}                                                     \
				successor->left = (NODE)->left;                       \
				if (successor->left)                                  \
					successor->left->parent = successor;              \
				successor->parent = parent;                           \
				if (parent == NULL)                                   \
					TABLE_ROOT = successor;                           \
				else if (parent->left == (NODE))                      \
					parent->left = successor;                         \
				else                                                  \
					parent->right = successor;                        \
			}                                                         \
		}                                                             \
		ret;                                                          \
	})

#endif
