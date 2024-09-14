#ifndef FAYT_ALLOCATOR_HPP_
#define FAYT_ALLOCATOR_HPP_

#include <fayt/stream.hpp>

namespace fayt {

constexpr int PAGE_SIZE = 0x1000;

class PageFrameAllocator {
public:
	PageFrameAllocator(uintptr_t base, size_t limit) : holeRoot{0, 0, nullptr, nullptr},
		holeTail(nullptr), base(base), limit(limit), current(base) { }

	void Calibrate(uintptr_t base, uintptr_t limit) {
		this->base = base; 
		this->limit = limit;
		this->current = base;
	}

	void *AllocPages(size_t cnt) {
		uintptr_t address = AllocAddress(cnt * PAGE_SIZE);
		if(!address) return nullptr;

		PortalReq portalReq = {
			.type = PORTAL_REQ_ANON,
			.prot = PORTAL_PROT_READ | PORTAL_PROT_WRITE,
			.length = sizeof(PortalReq),
			.share = {
				.identifier = NULL, 
				.type = 0,
				.create = 0 
			},
			.morphology = {
				.addr = address,
				.length = cnt * PAGE_SIZE 
			}
		};

		PortalResp portalResp;

		SyscallRet ret = syscall(SYSCALL_PORTAL, &portalReq, &portalResp);
		if(ret.ret == -1 || portalResp.base != address ||
			portalResp.limit != cnt * PAGE_SIZE) return nullptr;

		return reinterpret_cast<void*>(portalResp.base);
	}

	void insertHole(uintptr_t base, size_t limit) {
		if(holeTail == nullptr) {
			holeRoot.base = base;
			holeRoot.limit = limit;
			holeRoot.next = nullptr;

			holeTail = &holeRoot;

			return;
		}

		Hole *hole = new Hole;

		hole->base = base;
		hole->limit = limit;
		hole->next = nullptr;
		hole->last = holeTail;

		holeTail->next = hole;
		holeTail = hole;
	}

	void deleteHole(uintptr_t base, size_t limit) {
		Hole *hole = &holeRoot;

		for(;hole;) { 
			if(base >= hole->base + hole->limit || hole->base >= base + limit) {
				continue;
			}

			if(base == hole->base && limit == hole->limit) {
				hole->last->next = hole->next;
				hole->next->last = hole->last;
				delete hole;

				return;
			}

			// TODO handle fractional splits

			hole = hole->next;
		}
	}

	uintptr_t AllocAddress(size_t size) {
		if(current + size > (base + limit)) return 0;

		uintptr_t address = current;
check:
		Hole *hole = &this->holeRoot;

		for(;hole;) {
			if((address < hole->base + hole->limit) && (hole->base < address + size)) {
				address += hole->limit;
				goto check;
			}
			hole = hole->next;
		}

		current += size;

		return address;
	}
private:
	struct Hole {
		uintptr_t base;
		uintptr_t limit;
		Hole *next;
		Hole *last;
	};

	struct Hole holeRoot;
	struct Hole *holeTail;

	uintptr_t base;
	uintptr_t limit;
	uintptr_t current;
};

}

#endif
