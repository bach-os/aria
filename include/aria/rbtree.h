#ifndef ARIA_RBTREE_H_
#define ARIA_RBTREE_H_
#include <aria/bst.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * Inserts `elem` in `tree` using the Red-Black policy
 */
void rb_insert(struct bst *tree, struct bst_node *elem);

/**
  * Removes `elem` from`tree` using the Red-Black policy
 */
void rb_delete(struct bst *tree, struct bst_node *elem);

#ifdef __cplusplus
}
#endif

#endif