#ifndef FAYT_BITS_HPP_
#define FAYT_BITS_HPP_

#include <cstdint>
#include <cstddef> 

namespace fayt {

constexpr size_t div_roundup(size_t a, size_t b) {
    return (a + b - 1) / b;
}

constexpr size_t align_up(size_t a, size_t b) {
    return div_roundup(a, b) * b;
}

template <typename T, size_t N>
constexpr size_t length_of(const T(&)[N]) {
    return N;
}

template <typename T>
constexpr size_t abs_diff(T a, T b) {
    return (a > b) ? (a - b) : (b - a);
}

template <typename T>
constexpr void bit_set(T* a, size_t b) {
    a[b / 8] |= (1 << (b % 8));
}

template <typename T>
constexpr void bit_clear(T* a, size_t b) {
    a[b / 8] &= ~(1 << (b % 8));
}

template <typename T>
constexpr bool bit_test(const T* a, size_t b) {
    return ((a[b / 8] >> (b % 8)) & 0x1) != 0;
}

constexpr inline void memset8(uint8_t *src, uint8_t data, size_t n) {
	for(size_t i = 0; i < n; i++) {
		*src++ = data;
	}
}

constexpr inline void memset16(uint16_t *src, uint16_t data, size_t n) {
	for(size_t i = 0; i < n; i++) {
		*src++ = data;
	}
}

constexpr inline void memset32(uint32_t *src, uint32_t data, size_t n) {
	for(size_t i = 0; i < n; i++) {
		*src++ = data;
	}
}

constexpr inline void memset64(uint64_t *src, uint64_t data, size_t n) {
	for(size_t i = 0; i < n; i++) {
		*src++ = data;
	}
}

constexpr inline void memcpy8(uint8_t *dest, const uint8_t *src, size_t n) {
	for(size_t i = 0; i < n; i++) {
		dest[i] = src[i];
	}
}

constexpr inline void memcpy16(uint16_t *dest, const uint16_t *src, size_t n) {
	for(size_t i = 0; i < n; i++) {
		*dest++ = *src++;
	}
}

constexpr inline void memcpy32(uint32_t *dest, const uint32_t *src, size_t n) {
	for(size_t i = 0; i < n; i++) {
		*dest++ = *src++;
	}
}

constexpr inline void memcpy64(uint64_t *dest, const uint64_t *src, size_t n) {
	for(size_t i = 0; i < n; i++) {
		*dest++ = *src++;
	}
}

constexpr inline void memcpy(void *dest, const void *src, size_t n) {
	memcpy8(reinterpret_cast<uint8_t*>(dest), reinterpret_cast<const uint8_t*>(src), n);
}

constexpr inline void memset(void *src, int data, size_t n) {
	memset8(reinterpret_cast<uint8_t*>(src), data, n);
}

}

#endif
