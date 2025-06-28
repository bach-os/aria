#ifndef ARIA_SCHED_H_
#define ARIA_SCHED_H_

#include <aria/capability.h>
#include <aria/bitmap.h>
#include <aria/dictionary.h>
#include <aria/time.h>

constexpr int ARCHCTL_SCHED_ACQUIRE = 1;
constexpr int ARCHCTL_SCHED_RELEASE = 2;
constexpr int ARCHCTL_RESERVE_IRQ = 3;
constexpr int ARCHCTL_RELEASE_IRQ = 4;
constexpr int ARCHCTL_YIELD = 5;

constexpr int FUTEX_WAIT = 1;
constexpr int FUTEX_WAKE = 2;

#endif
