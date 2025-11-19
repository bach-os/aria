#ifndef ARIA_AVL_H_
#define ARIA_AVL_H_
#include <aria/bst.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * Inserts `elem` in `tree` using the AVL policy
 */
void avl_insert(struct bst *tree, struct bst_node *elem);

/**
  * Removes `elem` from`tree` using the AVL policy
 */
void avl_delete(struct bst *tree, struct bst_node *elem);

#ifdef __cplusplus
}
#endif

#endif
