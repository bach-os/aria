#ifndef FAYT_CONTAINER_OF_
#define FAYT_CONTAINER_OF_

#define CONTAINER_OF(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif
