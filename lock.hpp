#ifndef FAYT_LOCK_HPP_
#define FAYT_LOCK_HPP_

#include <atomic>

namespace fayt {

class Spinlock {
public:
	Spinlock() : _lock(false), raw(nullptr) { }
	Spinlock(char *raw) : _lock(false), raw(raw) { }

	void lock() {
		if(raw) {
			while(__atomic_test_and_set(raw, __ATOMIC_ACQUIRE));
			return;
		} 

		bool expected = false;
		while (!_lock.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
			expected = false; 
		}
	}

	void unlock() {
		_lock.store(false, std::memory_order_release);
	}

private:
	std::atomic<bool> _lock;
	char *raw;
};

class SpinlockGuard {
public:
	explicit SpinlockGuard(Spinlock &spinlock) : spin_lock(spinlock) {
		spin_lock.lock();
	}

	~SpinlockGuard() {
		spin_lock.unlock();
	}
private:
	Spinlock &spin_lock;
};

}

#endif
