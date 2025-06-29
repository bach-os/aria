#ifndef ARIA_DEBUG_H_
#define ARIA_DEBUG_H_

void print(const char *str, ...);
void panic(const char *str, ...);

#define RETURN_ERROR                                                          \
	({                                                                        \
		print("aria: warning: function <%s> within <%s> failed on line=%d\n", \
			  __FILE__, __func__, __LINE__);                                  \
		return -1;                                                            \
	})

#define REPORT_ERROR                                                                  \
	({                                                                                \
		print(                                                                        \
			"aria: reporting problem: function <%s> within <%s> failed on line=%d\n", \
			__FILE__, __func__, __LINE__);                                            \
	})

#define _STRINGIFY(X) #X
#define STRINGIFY(X) _STRINGIFY(X)

#define ASSERT(EXPR)                                            \
	({                                                          \
		if (!(EXPR)) {                                          \
			panic("Assertion \"" #EXPR "\" failed at " __FILE__ \
				  ":" STRINGIFY(__LINE__) " in %s",             \
				  __func__);                                    \
		}                                                       \
	})

#endif
