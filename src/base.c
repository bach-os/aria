#include <aria/base.h>
#include <aria/compiler.h>

int strcmp(const char *str0, const char *str1)
{
	for (size_t i = 0;; i++) {
		if (str0[i] != str1[i]) {
			return str0[i] - str1[i];
		}

		if (!str0[i]) {
			return 0;
		}
	}
}

int strncmp(const char *str0, const char *str1, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (str0[i] != str1[i]) {
			return str0[i] - str1[i];
		}

		if (!str0[i]) {
			return 0;
		}
	}

	return 0;
}

char *strcpy(char *dest, const char *src)
{
	size_t i = 0;

	for (; src[i]; i++) {
		dest[i] = src[i];
	}

	dest[i] = 0;

	return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	size_t i = 0;

	for (; i < n && src[i]; i++) {
		dest[i] = src[i];
	}

	dest[i] = 0;

	return dest;
}

char *strchr(const char *str, int c)
{
	while (*str++) {
		if (*str == c) {
			return (char *)str;
		}
	}
	return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t *c1 = (const uint8_t *)s1;
	const uint8_t *c2 = (const uint8_t *)s2;

	for (size_t i = 0; i < n; i++) {
		if (c1[i] != c2[i]) {
			return c1[i] - c2[i];
		}
	}

	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	return memcpy8(dest, src, n);
}

void *memset(void *src, int data, size_t n)
{
	return memset8(src, data, n);
}
