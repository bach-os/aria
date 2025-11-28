#ifndef ARIA_LCG_H_
#define ARIA_LCG_H_
#include <stdint.h>

/**
 * Linear Congruential Generator
 */
struct lcg {
	uint64_t m; /**< Modulus */
	uint64_t a; /**< Multiplier */
	uint64_t c; /**< Increment */
	uint64_t seed; /**< Seed */
};

static inline void lcg_init(struct lcg *lcg, uint64_t m, uint64_t a, uint64_t c,
							uint64_t seed)
{
	lcg->m = m;
	lcg->a = a;
	lcg->c = c;
	lcg->seed = seed;
}

/**
  * Returns a pseudo-random value from `lcg`
*/
static inline uint64_t lcg_rand(struct lcg *lcg)
{
	lcg->seed = (lcg->a * lcg->seed + lcg->c) % lcg->m;
	return lcg->seed;
}

#endif
