#ifndef FAYT_CIRCULAR_QUEUE_HPP_
#define FAYT_CIRCULAR_QUEUE_HPP_

#include <cstdint>
#include <cstddef>
#include <cstdbool>

#include <fayt/bits.hpp>

namespace fayt {

class CircularQueue {
public:
	CircularQueue(void *data, size_t size, size_t obj_size) : size(size), obj_size(obj_size),
	head(-1), tail(-1), items(0) {
		if (data) this->data = data;
		else this->data = new char[size * obj_size];
	}

	~CircularQueue() {
		if(data) delete[] static_cast<char*>(data);
	}

	bool push(const void *item) {
		if((head == 0 && tail == (size - 1)) || (head == (tail + 1))) return false;

		if(head == -1) {
			head = 0;
			tail = 0;
		} else {
			if(tail >= (size - 1)) tail = 0;
			else tail++;
		}

		memcpy(static_cast<char*>(data) + (tail * obj_size), item, obj_size);
		__atomic_add_fetch(&items, 1, __ATOMIC_RELAXED); 

		return true;
	}

	bool pop(void *item) {
		if(head == -1) return false;

		memcpy(item, static_cast<char*>(data) + (head * obj_size), obj_size);
		__atomic_sub_fetch(&items, 1, __ATOMIC_RELAXED);

		if(head == tail) {
			head = -1;
			tail = -1;
		} else	{
			if(head == (size - 1)) head = 0;
			else head++;
		}

		return true;
	}

	bool pop_tail(void *item) {
		if (head == tail) return false;

		if(tail <= 0) tail = size - 1;
		else tail--;

		memcpy(item, static_cast<char*>(data) + (tail * obj_size), obj_size);
		__atomic_sub_fetch(&items, 1, __ATOMIC_RELAXED);

		return true;
	}

	bool peek(void *item) const {
		if (head == -1) return false;

		memcpy(item, static_cast<char*>(data) + (head * obj_size), obj_size);

		return true;
	}

private:
	void *data;
	int size;
	int obj_size;
	int head;
	int tail;
	size_t items;
};

}

#endif
