#ifndef FAYT_NOTIFICATION_HPP_
#define FAYT_NOTIFICATION_HPP_

namespace fayt {

struct NotificationInfo {

}; 
	
struct NotificationAction {
	void (*handler)(struct NotificationInfo*, void*, int);
};

}

#endif
