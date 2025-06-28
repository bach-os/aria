#ifndef ARIA_ADDRESS_H_
#define ARIA_ADDRESS_H_

#include <aria/capability.h>

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 0x1000ull

enum { AS_ACTION_CONSTRUCT, AS_ACTION_ALLOCATE, AS_ACTION_FREE };

int as_vmem_allocate(capability_t capability, uintptr_t *, size_t);
int as_vmem_free(capability_t capability, uintptr_t, size_t);
int as_mem_allocate(capability_t capability, uintptr_t *, size_t);
int as_mem_free(capability_t capability, uintptr_t, size_t);

#endif
