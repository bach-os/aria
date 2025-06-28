#ifndef ARIA_HANDLE_H_
#define ARIA_HANDLE_H_

#include <stdint.h>

#define CAPABILITY_ACCESS_READ (1 << 0)
#define CAPABILITY_ACCESS_WRITE (1 << 1)

typedef uint32_t capability_t;

enum { CAPABILITY_SELF_THREAD, CAPABILITY_SELF_AS };

#endif
