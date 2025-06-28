#include <aria/notification.h>
#include <aria/syscall.h>
#include <aria/address.h>
#include <aria/debug.h>
#include <aria/string.h>

#ifndef DUFAY

int notify(struct comm_bridge *bridge)
{
	if (bridge == NULL)
		return -1;

	//uintptr_t vaddr;
	//int ret =
	//	as_vmem_allocate(&address_space, &vaddr,
	//				DIV_ROUNDUP(bridge->data.limit, PAGE_SIZE) * PAGE_SIZE);
	//if (ret == -1)
	//	return -1;

	//void *tmp = bridge->data.base;
	//bridge->data.base = (void *)vaddr;

	//struct syscall_response response =
	//	SYSCALL1(SYSCALL_NOTIFICATION_BUILD, bridge);
	//if (response.ret == -1)
	//	return -1;

	//memcpy((void *)bridge->data.base, tmp, bridge->data.limit);

	//response = SYSCALL1(SYSCALL_NOTIFICATION_BROADCAST, bridge);
	//if (response.ret == -1)
	//	return -1;

	return 0;
}

#endif
