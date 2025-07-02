#include <benchmark/benchmark.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <aria/hamt.h>
#include <unordered_map>
#include <frg/hash_map.hpp>
#include <absl/container/flat_hash_map.h>

void my_free(void *ptr, size_t size)
{
	(void)size;
	free(ptr);
}

bool cmp(void *a, void *b)
{
	return strcmp((char *)a, (char *)b) == 0;
}

static uint64_t murmur_hash(void *key, size_t len)
{
	const uint64_t m = 0xc6a4a7935bd1e995LLU;
	const int r = 47;
	uint64_t h = (0xC001C0DE) ^ (len * m);
	const uint64_t *data = (const uint64_t *)key;
	const uint64_t *end = (len >> 3) + data;
	while (data != end) {
		uint64_t k = *data++;
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	const unsigned char *data2 = (const unsigned char *)data;
	switch (len & 7) {
	case 7:
		h ^= (uint64_t)(data2[6]) << 48;
		/* fall through */
	case 6:
		h ^= (uint64_t)(data2[5]) << 40;
		/* fall through */
	case 5:
		h ^= (uint64_t)(data2[4]) << 32;
		/* fall through */
	case 4:
		h ^= (uint64_t)(data2[3]) << 24;
		/* fall through */
	case 3:
		h ^= (uint64_t)(data2[2]) << 16;
		/* fall through */
	case 2:
		h ^= (uint64_t)(data2[1]) << 8;
		/* fall through */
	case 1:
		h ^= (uint64_t)(data2[0]);
		h *= m;
	default:
		break;
	};
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

extern "C" void print(const char *, ...)
{
}

uint64_t my_hash(void *key)
{
	return murmur_hash(key, strlen((char *)key));
}

template <typename T> struct MyHash {};

template <> struct MyHash<const char *> {
	uint64_t operator()(const char *key) const
	{
		return murmur_hash((void *)key, strlen((char *)key));
	}
};

template <> struct MyHash<std::string> {
	uint64_t operator()(std::string key) const
	{
		return murmur_hash((void *)key.c_str(), key.size());
	}
};

static void BM_HAMT(benchmark::State &state)
{
	struct hamt h;
	allocator_t alloc = { malloc, my_free, NULL };
	FILE *f;
	char *line = NULL;
	size_t len = 0;
	size_t i = 0;

	hamt_init(&h, alloc, cmp, my_hash);

	f = fopen("words.txt", "r");

	std::vector<std::string> lines;

	while (getline(&line, &len, f) && i < 500000) {
		if (!len)
			continue;

		lines.push_back(line);
		i++;
	}

	// Perform setup here
	for (auto _ : state) {
		for (auto line : lines) {
			size_t *mlen = (size_t *)malloc(sizeof(size_t));
			*mlen = line.size();
			hamt_insert(&h, (void *)line.c_str(), mlen);
			size_t *got = (size_t *)hamt_get(&h, (void *)line.c_str());

			if (*got != *mlen) {
				printf("failed on %s (%ld != %ld)\n", line.c_str(), *mlen,
					   *got);
			}
		}

		for (auto line : lines) {
			hamt_remove(&h, (void *)line.c_str());
		}
	}
}

static void BM_std(benchmark::State &state)
{
	FILE *f;
	char *line = NULL;
	size_t len = 0;
	size_t i = 0;

	f = fopen("words.txt", "r");

	std::unordered_map<const char *, size_t *, MyHash<const char *> > m;

	std::vector<std::string> lines;

	while (getline(&line, &len, f) && i < 500000) {
		if (!len)
			continue;

		lines.push_back(line);
		i++;
	}

	// Perform setup here
	for (auto _ : state) {
		for (auto line : lines) {
			size_t *mlen = (size_t *)malloc(sizeof(size_t));
			*mlen = line.size();
			m.insert({ line.c_str(), mlen });
			size_t *got = m[line.c_str()];

			if (*got != *mlen) {
				printf("failed on %s (%ld != %ld)\n", line.c_str(), *mlen,
					   *got);
			}
		}

		for (auto line : lines) {
			m.erase(line.c_str());
		}
	}
}

static void BM_abseil(benchmark::State &state)
{
	FILE *f;
	char *line = NULL;
	size_t len = 0;
	size_t i = 0;

	f = fopen("words.txt", "r");

	absl::flat_hash_map<const char *, size_t *> m;

	std::vector<std::string> lines;

	while (getline(&line, &len, f) && i < 500000) {
		if (!len)
			continue;

		lines.push_back(line);
		i++;
	}

	// Perform setup here
	for (auto _ : state) {
		for (auto line : lines) {
			size_t *mlen = (size_t *)malloc(sizeof(size_t));
			*mlen = line.size();
			m[line.c_str()] = mlen;
			size_t *got = m[line.c_str()];

			if (*got != *mlen) {
				printf("failed on %s (%ld != %ld)\n", line.c_str(), *mlen,
					   *got);
			}
		}

		for (auto line : lines) {
			m.erase(line.c_str());
		}
	}
}

struct MyAlloc {
	void *allocate(size_t size)
	{
		return malloc(size);
	}
	void deallocate(void *p, size_t size)
	{
		free(p);
		(void)size;
	}
};

static void BM_frigg(benchmark::State &state)
{
	FILE *f;
	char *line = NULL;
	size_t len = 0;
	size_t i = 0;
	frg::hash_map<const char *, size_t *, MyHash<const char *>, MyAlloc> h(
		MyHash<const char *>{});

	f = fopen("words.txt", "r");

	std::vector<std::string> lines;

	while (getline(&line, &len, f) && i < 500000) {
		if (!len)
			continue;

		line[len - 1] = 0;

		lines.push_back(line);

		i++;
	}

	// Perform setup here
	for (auto _ : state) {
		for (auto &line : lines) {
			size_t *mlen = (size_t *)malloc(sizeof(size_t));
			*mlen = line.size();
			h.insert(line.c_str(), mlen);
			size_t *got = *h.get(line.c_str());
			if (*got != *mlen) {
				printf("failed on %s (%ld != %ld)\n", line.c_str(), *mlen,
					   *got);
			}
		}

		for (auto &line : lines) {
			h.remove(line.c_str());
		}
	}
}

BENCHMARK(BM_HAMT);
BENCHMARK(BM_frigg);
BENCHMARK(BM_std);
BENCHMARK(BM_abseil);
BENCHMARK_MAIN();
