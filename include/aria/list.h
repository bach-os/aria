#ifndef ARIA_LIST_H_
#define ARIA_LIST_H_
#include <stddef.h>
#include <aria/base.h>

/**
 * Branchless circular list
*/
struct list_entry {
	struct list_entry *next;
	struct list_entry *prev;
};

#define LIST_INITIALIZER(HEAD) { .next = &(HEAD), .prev = &(HEAD) }

/**
 * Initializes the list head `head`
*/
ALWAYS_INLINE static inline void list_init(struct list_entry *head)
{
	head->next = head;
	head->prev = head;
}

/**
 * Inserts an element at the end of the list
*/
ALWAYS_INLINE static inline void list_insert_tail(struct list_entry *head,
												  struct list_entry *elem)
{
	head->prev->next = elem;
	elem->next = head;
	elem->prev = head->prev;
	head->prev = elem;
}

/**
 * Inserts an element at the beginning of the list
*/
ALWAYS_INLINE static inline void list_insert_head(struct list_entry *head,
												  struct list_entry *elem)
{
	head->next->prev = elem;
	elem->next = head->next;
	elem->prev = head;
	head->next = elem;
}

/**
 * Removes an element from the list
*/
ALWAYS_INLINE static inline void list_remove(struct list_entry *elem)
{
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
}

/**
 * Returns whether or not the list is empty
*/
ALWAYS_INLINE static inline bool list_empty(struct list_entry *head)
{
	return (head->next == head);
}

/**
 * Returns the first element of a list
*/
ALWAYS_INLINE static inline struct list_entry *
list_first(struct list_entry *head)
{
	return (head->next);
}

/**
 * Returns the last element of a list (tail)
*/
ALWAYS_INLINE static inline struct list_entry *
list_tail(struct list_entry *head)
{
	return (head->prev);
}

#define LIST_ENTRY(PTR, TYPE, FIELD) \
	((TYPE *)((char *)(PTR) - offsetof(TYPE, FIELD)))

#define LIST_FOREACH(VAR, HEAD, FIELD)                            \
	for ((VAR) = LIST_ENTRY((HEAD)->next, typeof(*(VAR)), FIELD); \
		 &(VAR)->FIELD != (HEAD);                                 \
		 (VAR) = LIST_ENTRY((VAR)->FIELD.next, typeof(*(VAR)), FIELD))

#endif
