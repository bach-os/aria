#ifndef FAYT_COMPILER_HPP_
#define FAYT_COMPILER_HPP_

#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)

#endif
