#ifndef ARIA_NOTIFICATION_H_
#define ARIA_NOTIFICATION_H_

#include <aria/capability.h>
#include <aria/sched.h>

constexpr int NOTIFY_WEIGHT_SCHEDULED = (1 << 0);
constexpr int NOTIFY_WEIGHT_TICK = (1 << 1);
constexpr int NOTIFY_WEIGHT_INSTANTANEOUS = (1 << 2);

struct [[gnu::packed]] comm_bridge {
	int not;
	int weight;

	struct {
		void *base;
		int limit;
	} data;

	capability_t destination;

	int lnkidx;
};

struct notification_info {
	void *private;
};

struct notification_action {
	void (*handler)(struct notification_info *, void *, int);
};

int notify(struct comm_bridge *);

#endif
