#ifndef ARIA_DICTIONARY_H_
#define ARIA_DICTIONARY_H_

#include <stddef.h>
#include <stdint.h>

struct dictionary {
	void **keys;
	void **data;

	size_t capacity;
	size_t element_cnt;
};

int dictionary_push(struct dictionary *, void *, void *, size_t);
int dictionary_delete(struct dictionary *, void *, size_t);
int dictionary_destroy(struct dictionary *);
int dictionary_search(struct dictionary *, void *, size_t, void **);

#endif
