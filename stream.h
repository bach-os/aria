#ifndef ARIA_STREAM_H_
#define ARIA_STREAM_H_

#include <aria/lock.h>
#include <stdarg.h>

struct stream_info {
	void *private;
	void (*write)(struct stream_info *, char);
	struct spinlock lock;
};

int stream_print(struct stream_info *stream, const char *str, va_list arg);

#endif
