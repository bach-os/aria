#ifndef FAYT_SLAB_HPP
#define FAYT_SLAB_HPP

#include <fayt/lock.hpp>
#include <fayt/bits.hpp>
#include <fayt/allocator.hpp>

#include <new>

namespace fayt {

constexpr int OBJECTS_PER_SLAB = 512;

template <typename A, typename T>
class Slab;

template <typename A, typename T>
class Cache {
public:
	Cache() : name(nullptr), Allocator(nullptr) { }

	Cache(A *Allocator, const char *name) : name(name), Allocator(Allocator) {
		this->pagesPerSlab = div_roundup(sizeof(T) * OBJECTS_PER_SLAB +
			sizeof(Slab<A, T>) + OBJECTS_PER_SLAB, PAGE_SIZE);

		slabEmpty = new (Allocator->AllocPages(this->pagesPerSlab)) Slab<A, T>(this);
	} 

	int getObjectSize(void) const { return objectSize; }
	int getActiveSlabs(void) const { return activeSlabs; }
	int getPagesPerSlab(void) const { return pagesPerSlab; }

	void *AllocObject(void) {
		Slab<A, T> *slab = nullptr;

		{
			SpinlockGuard guard(this->lock);

			if(this->slabPartial) {
				slab = this->slabPartial;
			} else if(this->slabEmpty) {
				slab = this->slabEmpty;
			} else {
				slab = new (Allocator->AllocPages(this->pagesPerSlab)) Slab<A, T>(this);
				this->slabEmpty = slab;
			}
		}

		void *addr = slab->AllocObject();

		if(slab->getAvailableObjects() == 0) {
			this->moveSlab(&this->slabFull, &this->slabPartial, slab);
		} else if(slab->getAvailableObjects() == (slab->getTotalObjects() - 1)) {
			this->moveSlab(&this->slabPartial, &this->slabEmpty, slab);
		}

		return addr;
	}

	int FreeObject(void *obj) {
		SpinlockGuard guard(this->lock);

		Slab<A, T> *slab = slabPartial;
		bool checkAgain = true;
check_slab:
		while(slab) {
			int ret = slab->FreeObject(obj);
			if(ret == 0) return 0;
			slab = slab->next;
		}

		if(checkAgain) {
			checkAgain = false; slab = slabFull;
			goto check_slab;
		}

		return -1;
	}
protected:
	friend class Slab<A, T>;

	Slab<A, T> *slabEmpty;
	Slab<A, T> *slabPartial;
	Slab<A, T> *slabFull;
private:
	int moveSlab(Slab<A, T> **destHead, Slab<A, T> **srcHead, Slab<A, T> *src) {
		if(!src || !*srcHead) return -1; 
		if(src->next != NULL) src->next->last = src->last;
		if(src->last != NULL) src->last->next = src->next;
		if(*srcHead == src) *srcHead = src->next;

		if(!*destHead) {
			src->last = NULL;
			src->next = NULL;

			*destHead = src; 

			return 0;
		}

		src->next = *destHead;
		src->last = NULL;

		if(*destHead) {
			(*destHead)->last = src;
		}

		*destHead = src;

		return 0;
	}

	int objectSize;
	int activeSlabs;
	int pagesPerSlab;

	const char *name;

	A *Allocator;
	Spinlock lock;
};

template <typename A, typename T>
class Slab {
public:
	Slab(Cache<A, T> *cache) : bitmap(nullptr), buffer(nullptr), availableObjects(OBJECTS_PER_SLAB), totalObjects(OBJECTS_PER_SLAB), cache(cache) {
		bitmap = reinterpret_cast<char*>(this + 1);
		buffer = reinterpret_cast<void*>(align_up(reinterpret_cast<uintptr_t>(this->bitmap) +
			OBJECTS_PER_SLAB, 16));
	}

	int getAvailableObjects(void) const { return availableObjects; }
	int getTotalObjects(void) const { return totalObjects; }

	void *AllocObject(void) {
		SpinlockGuard guard(this->lock);

		for(int i = 0; i < this->totalObjects; i++) {
			if(!bit_test(this->bitmap, i)) {
				bit_set(this->bitmap, i);
				this->availableObjects--;

				memset(reinterpret_cast<char*>(this->buffer) + (i * this->cache->getObjectSize()), 0, this->cache->getObjectSize());

				return reinterpret_cast<char*>(this->buffer) + (i * this->cache->getObjectSize());
			}
		}

		return nullptr;
	}
	
	int FreeObject(void *obj) {
		SpinlockGuard guard(this->lock);

		if(reinterpret_cast<uintptr_t>(this->buffer) <= reinterpret_cast<uintptr_t>(obj) &&
				reinterpret_cast<uintptr_t>(obj) < (reinterpret_cast<uintptr_t>(this->buffer) +
				sizeof(T) * this->totalObjects)) {
			size_t index = (reinterpret_cast<uintptr_t>(obj) - reinterpret_cast<uintptr_t>(this->buffer)) / sizeof(T);

			if (bit_test(this->bitmap, index)) {
				bit_clear(this->bitmap, index);
				this->availableObjects++;
				return 0;
			}
		}
		return -1;
	}
protected:
	friend class Cache<A, T>;

	Slab<A, T> *next;
	Slab<A, T> *last;
private:
	char *bitmap;
	void *buffer;

	int availableObjects;
	int totalObjects;

	Cache<A, T> *cache;

	Spinlock lock;
};

template <typename A>
class CacheDirectory {
public:
	CacheDirectory(A *Allocator) : Allocator(Allocator) {
		new (&cache32) Cache<A, _32>(Allocator, "32 BYTE");
		new (&cache64) Cache<A, _64>(Allocator, "64 BYTE");
		new (&cache128) Cache<A, _128>(Allocator, "128 BYTE");
		new (&cache256) Cache<A, _256>(Allocator, "256 BYTE");
		new (&cache512) Cache<A, _512>(Allocator, "512 BYTE");
		new (&cache1024) Cache<A, _1024>(Allocator, "1024 BYTE");
		new (&cache2048) Cache<A, _2048>(Allocator, "2048 BYTE");
		new (&cache4096) Cache<A, _4096>(Allocator, "4096 BYTE");
		new (&cache8192) Cache<A, _8192>(Allocator, "8192 BYTE");
		new (&cache16384) Cache<A, _16384>(Allocator, "16384 BYTE");
		new (&cache32768) Cache<A, _32768>(Allocator, "32768 BYTE");
	}

	template <typename T>
	Cache<A, T> *CreateCache(const char *name) {
		return new (Allocator->AllocPages(1)) Cache<A, T>(Allocator, name);
	}

	void *AllocObject(int size) {
		int roundedSize = [](int size) -> int {
			int roundedSize = 1;
			while(roundedSize < size) roundedSize *= 2;
			if(size <= 16) return 32;
			return roundedSize;
		} (size);

		if (roundedSize == 32) {
			return cache32.AllocObject();
		} else if (roundedSize == 64) {
			return cache64.AllocObject();
		} else if (roundedSize == 128) {
			return cache128.AllocObject();
		} else if (roundedSize == 256) {
			return cache256.AllocObject();
		} else if (roundedSize == 512) {
			return cache512.AllocObject();
		} else if (roundedSize == 1024) {
			return cache1024.AllocObject();
		} else if (roundedSize == 2048) {
			return cache2048.AllocObject();
		} else if (roundedSize == 4096) {
			return cache4096.AllocObject();
		} else if (roundedSize == 8192) {
			return cache8192.AllocObject();
		} else if (roundedSize == 16384) {
			return cache16384.AllocObject();
		} else if (roundedSize == 32768) {
			return cache32768.AllocObject();
		} else {
			return nullptr;
		}

		return nullptr;
	}

	void FreeObject(void *obj) {
		if(cache32.FreeObject(obj) != 0) return;
		if(cache64.FreeObject(obj) != 0) return;
		if(cache128.FreeObject(obj) != 0) return;
		if(cache256.FreeObject(obj) != 0) return;
		if(cache512.FreeObject(obj) != 0) return;
		if(cache1024.FreeObject(obj) != 0) return;
		if(cache2048.FreeObject(obj) != 0) return;
		if(cache4096.FreeObject(obj) != 0) return;
		if(cache8192.FreeObject(obj) != 0) return;
		if(cache16384.FreeObject(obj) != 0) return;
		if(cache32768.FreeObject(obj) != 0) return;
	}
private:
	struct _32 { alignas(1) uint8_t data[32]; };
	struct _64 { alignas(1) uint8_t data[64]; };
	struct _128 { alignas(1) uint8_t data[128]; };
	struct _256 { alignas(1) uint8_t data[256]; };
	struct _512 { alignas(1) uint8_t data[512]; };
	struct _1024 { alignas(1) uint8_t data[1024]; };
	struct _2048 { alignas(1) uint8_t data[2048]; };
	struct _4096 { alignas(1) uint8_t data[4096]; };
	struct _8192 { alignas(1) uint8_t data[8192]; };
	struct _16384 { alignas(1) uint8_t data[16384]; };
	struct _32768 { alignas(1) uint8_t data[32768]; };

	Cache<A, _32> cache32;
	Cache<A, _64> cache64;
	Cache<A, _128> cache128;
	Cache<A, _256> cache256;
	Cache<A, _512> cache512;
	Cache<A, _1024> cache1024;
	Cache<A, _2048> cache2048;
	Cache<A, _4096> cache4096;
	Cache<A, _8192> cache8192;
	Cache<A, _16384> cache16384; 
	Cache<A, _32768> cache32768;

	A *Allocator;
};

}

#endif
