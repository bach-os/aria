#include <aria/address.h>
#include <aria/syscall.h>
#include <aria/debug.h>
#include <aria/portal.h>

int as_mem_allocate(capability_t capability, uintptr_t *address, size_t length)
{
	int ret = as_vmem_allocate(capability, address, length);
	if (ret == -1)
		RETURN_ERROR;

	struct portal_req portal_req = {
		.type = PORTAL_REQ_ANON,
		.prot = PORTAL_PROT_READ | PORTAL_PROT_WRITE,
		.length = sizeof(struct portal_req),
		.share = { .identifier = NULL, .type = 0, .create = 0 },
		.morphology = { .addr = *address, .length = length }
	};

	struct portal_resp portal_resp;

	struct syscall_response syscall_response =
		SYSCALL2(SYSCALL_PORTAL, &portal_req, &portal_resp);
	if (syscall_response.ret == -1 || portal_resp.base != *address ||
		portal_resp.limit != length)
		RETURN_ERROR;

	return length;
}

int as_mem_free(capability_t capability, uintptr_t address, size_t length)
{
	int ret = as_vmem_free(capability, address, length);
	if (ret == -1)
		RETURN_ERROR;

	// PORTAL UNMAP

	return 0;
}

int as_vmem_allocate(capability_t capability, uintptr_t *address, size_t length)
{
	struct syscall_response syscall_response = SYSCALL4(
		SYSCALL_AS_ACTION, capability, AS_ACTION_ALLOCATE, address, length);
	if (syscall_response.ret == -1)
		RETURN_ERROR;
	return syscall_response.ret;
}

int as_vmem_free(capability_t capability, uintptr_t address, size_t length)
{
	struct syscall_response syscall_response = SYSCALL4(
		SYSCALL_AS_ACTION, capability, AS_ACTION_FREE, address, length);
	if (syscall_response.ret == -1)
		RETURN_ERROR;
	return syscall_response.ret;
}
