#ifndef FAYT_STREAM_HPP_
#define FAYT_STREAM_HPP_

#include <cstdint>
#include <cstddef>

#include <fayt/syscall.hpp>

namespace fayt {

namespace stream {

template <typename T>
struct formatter {
	template <typename out>
	static void format(out &output_format, T arg, [[maybe_unused]] int);
};

template <>
struct formatter<const char*> {
	template <typename out>
	static void format(out &output_format, const char *arg, [[maybe_unused]] int) {
		for(;*arg;) output_format.write(*arg++);
	}
};

template <>
struct formatter<char*> {
	template <typename out>
	static void format(out &output_format, char *arg, [[maybe_unused]] int) {
		for(;*arg;) output_format.write(*arg++);
	}
};

template <>
struct formatter<char> {
	template <typename out>
	static void format(out &output_format, char arg, [[maybe_unused]] int) {
		output_format.write(arg);
	}
};

#define INTEGER_IMPL(TYPE) \
	template<> \
	struct formatter<TYPE> { \
		template <typename out> \
		static void format(out &output_format, TYPE arg, [[maybe_unused]] int base) { \
			static char digits[] = "0123456789ABCDEF"; \
			static char buffer[50]; \
			char *str = &buffer[49]; \
			*str = '\0'; \
			do { \
				*--str = digits[arg % base]; \
				arg /= base; \
			} while(arg); \
			while(*str) output_format.write(*str++); \
		} \
	};

INTEGER_IMPL(int);
INTEGER_IMPL(unsigned int);

INTEGER_IMPL(short int);
INTEGER_IMPL(unsigned short int);

INTEGER_IMPL(long int);
INTEGER_IMPL(unsigned long int);

INTEGER_IMPL(long long int);
INTEGER_IMPL(unsigned long long int);

INTEGER_IMPL(unsigned char);

#define POINTER_IMPL(TYPE) \
	template<> \
	struct formatter<TYPE*> { \
		template <typename out> \
		static void format(out &output_format, TYPE* arg, [[maybe_unused]] int base) { \
			uintptr_t num = reinterpret_cast<uintptr_t>(arg); \
			static char digits[] = "0123456789ABCDEF"; \
			static char buffer[50]; \
			char *str = &buffer[49]; \
			*str = '\0'; \
			do { \
				*--str = digits[num % base]; \
				num /= base; \
			} while(num); \
			while(*str) output_format.write(*str++); \
		} \
	};

POINTER_IMPL(int)
POINTER_IMPL(unsigned int)

POINTER_IMPL(short int)
POINTER_IMPL(unsigned short int)

POINTER_IMPL(long int)
POINTER_IMPL(unsigned long int)

POINTER_IMPL(long long int)
POINTER_IMPL(unsigned long long int)

POINTER_IMPL(unsigned char)
POINTER_IMPL(void);

template <typename out, typename T>
void format(out &output_format, T arg, int base) {
	formatter<T>::format(output_format, arg, base);
}

template <typename out, typename ...Args>
void print_format(out &output_format, const char *str, Args && ...args) {
	([&](auto &arg) {
		while(*str) {
			if(*str == '{') {
				if(*(str + 1) == '}') {
					format(output_format, arg, 10);
					str += 2;
					return;
				} else if(*(str + 2) == '}') {
					switch(*(str + 1)) {
						case 'b':
							format(output_format, arg, 2);
							break;
						case 'd':
							format(output_format, arg, 10);
							break;
						case 'x':
							format(output_format, arg, 16);
							break;
						default:
							format(output_format, arg, 16);
					}
					str += 3;
					return;
				}
			}

			output_format.write(*str++);
		}
	} (args), ...);

	while(*str) output_format.write(*str++); 
}

struct log_formatter {
	void write(char c) {
		syscall(0, c);
	}
};

inline log_formatter _log_formatter;

}

template <typename ...Args>
void print(const char *str, Args && ...args) {
	stream::print_format<stream::log_formatter>(stream::_log_formatter, str, args...);
}

}

#endif
