#ifndef ARIA_MESSAGE_H_
#define ARIA_MESSAGE_H_

#include <aria/capability.h>

#include <stddef.h>
#include <stdint.h>

#include <sys/queue.h>

#define PORT_RIGHT_RECV (1 << 0)
#define PORT_RIGHT_SEND (1 << 1)
#define PORT_RIGHT_SEND_ONCE (1 << 2)

struct message_header {
	capability_t destination;
	capability_t reply;
	size_t size;
};

#endif
