#ifndef FAYT_PORTAL_HPP
#define FAYT_PORTAL_HPP

#include <cstdint>
#include <cstddef>

namespace fayt {

constexpr int PORTAL_REQ_SHARE = 1 << 0;
constexpr int PORTAL_REQ_DIRECT = 1 << 1;
constexpr int PORTAL_REQ_ANON = 1 << 2;
constexpr int PORTAL_REQ_COW = 1 << 3;
constexpr int PORTAL_REQ_SP = 1 << 4;

constexpr int PORTAL_RESP_FAILURE = 1 << 0;
constexpr int PORTAL_RESP_SUCCESS = 1 << 1;

constexpr int PORTAL_PROT_READ = 1 << 0;
constexpr int PORTAL_PROT_WRITE = 1 << 1;
constexpr int PORTAL_PROT_EXEC = 1 << 2;

constexpr int PORTAL_SHARE_TYPE_CIRCULAR = 1 << 0;

struct [[gnu::packed]] PortalShareMeta {
	int type;
	int prot;
	char lock;
	size_t length;
};

struct [[gnu::packed]] PortalReq {
	int type;
	int prot;
	int length;

	struct [[gnu::packed]] {
		const char *identifier;
		int type;
		int create;
	} share;

	struct [[gnu::packed]] {
		uintptr_t addr;
		size_t length;
		uint64_t paddr[];
	} morphology;
};

struct [[gnu::packed]] PortalResp {
	uintptr_t base;
	uint64_t limit;
	uint64_t flags;
};

template <typename T>
class Portal {
public:
	T push();
	T pop();	
private:
	T *data;
};

}

#endif
