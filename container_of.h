#ifndef ARIA_CONTAINER_OF_
#define ARIA_CONTAINER_OF_

#define CONTAINER_OF(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif
