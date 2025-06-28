#ifndef ARIA_DEBUG_H_
#define ARIA_DEBUG_H_

void print(const char *str, ...);
void panic(const char *str, ...);

#define RETURN_ERROR                                                           \
	({                                                                         \
		print("dufay: warning: function <%s> within <%s> failed on line=%d\n", \
			  __FILE__, __func__, __LINE__);                                   \
		return -1;                                                             \
	})

#define REPORT_ERROR                                                                   \
	({                                                                                 \
		print(                                                                         \
			"dufay: reporting problem: function <%s> within <%s> failed on line=%d\n", \
			__FILE__, __func__, __LINE__);                                             \
	})

#endif
