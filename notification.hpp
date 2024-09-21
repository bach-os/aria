#ifndef FAYT_NOTIFICATION_HPP_
#define FAYT_NOTIFICATION_HPP_

namespace fayt {
	
struct NotificationAction {
	void (*handler)(int);
};

}

#endif
