#ifndef SYSCALL_HPP_
#define SYSCALL_HPP_

#include <cstddef>
#include <cstdint>

#include <type_traits>

namespace fayt {

struct Syscall {
    size_t syscall_idx;
    size_t arg_cnt;
};

struct SyscallRet {
	int64_t ret;
};

constexpr int SYSCALL_LOG = 0;
constexpr int SYSCALL_PORTAL = 1;
constexpr int SYSCALL_YIELD = 2;
constexpr int SYSCALL_NOTIFICATION_ACTION = 3;
constexpr int SYSCALL_NOTIFICATION_DEFINE_STACK = 4;
constexpr int SYSCALL_NOTIFICATION_RETURN = 5;
constexpr int SYSCALL_NOTIFICATION_MUTE = 6;
constexpr int SYSCALL_NOTIFICATION_UNMUTE = 7;

inline constexpr Syscall SYSCALLS[] = {
    {SYSCALL_LOG, 1},
    {SYSCALL_PORTAL, 2},
    {SYSCALL_YIELD, 0},
    {SYSCALL_NOTIFICATION_ACTION, 3},
    {SYSCALL_NOTIFICATION_DEFINE_STACK, 2},
    {SYSCALL_NOTIFICATION_RETURN, 0},
    {SYSCALL_NOTIFICATION_MUTE, 0},
    {SYSCALL_NOTIFICATION_UNMUTE, 0}
};

template<typename... Args>
SyscallRet syscall(size_t syscall_idx, Args... args) {
    SyscallRet ret = {0};

    auto syscall_meta = [](size_t idx) -> const Syscall* {
        for(const auto& syscall : SYSCALLS) {
            if(syscall.syscall_idx == idx) {
                return &syscall;
            }
        }
        return nullptr;
    } (syscall_idx);

    if(syscall_meta == nullptr) return SyscallRet {-1};

	constexpr size_t arg_cnt = sizeof...(args);
	if(arg_cnt != syscall_meta->arg_cnt) return SyscallRet {-1};

	auto convert [[maybe_unused]] = [](auto arg) -> uint64_t {
		if constexpr (std::is_pointer_v<decltype(arg)>) return reinterpret_cast<uint64_t>(arg);
		else if constexpr (std::is_integral_v<decltype(arg)>) return static_cast<uint64_t>(arg);
		else static_assert(std::is_pointer_v<decltype(arg)> || 
			std::is_integral_v<decltype(arg)>, "Unsupported type");
	};

	uint64_t arg_array[] = { convert(args)... };

	switch(syscall_meta->arg_cnt) {
        case 0: {
            asm volatile (
                "syscall"
                : "=a" (ret.ret), "=d" (ret.ret)
                : "a" (syscall_idx)
                : "rcx", "r11", "memory"
            );
            break;
        }
        case 1: {
            asm volatile (
                "syscall"
                : "=a" (ret.ret), "=d" (ret.ret)
                : "a" (syscall_idx), "D" (arg_array[0])
                : "rcx", "r11", "memory"
            );
            break;
        }
        case 2: {
            asm volatile (
                "syscall"
                : "=a" (ret.ret), "=d" (ret.ret)
                : "a" (syscall_idx), "D" (arg_array[0]), "S" (arg_array[1])
                : "rcx", "r11", "memory"
            );
            break;
        }
        case 3: {
            asm volatile (
                "syscall"
                : "=a" (ret.ret), "=d" (ret.ret)
                : "a" (syscall_idx), "D" (arg_array[0]), "S" (arg_array[1]), "d" (arg_array[2])
                : "rcx", "r11", "memory"
            );
            break;
        }
		case 4: {
			register uint64_t arg3 asm("r10") = (uint64_t)arg_array[3]; 
			asm volatile ("syscall" 
						  : "=d"(ret.ret) 
						  : "a"(syscall_idx), "D"(arg_array[0]), "S"(arg_array[1]), "d"(arg_array[2]), 
							"r"(arg3) 
						  : "rcx", "r11", "memory"); 
			break;
		}
		case 5: {
			register uint64_t arg3 asm("r10") = (uint64_t)arg_array[3]; 
			register uint64_t arg4 asm("r8") = (uint64_t)arg_array[4]; 
			asm volatile ("syscall" 
						  : "=d"(ret.ret) 
						  : "a"(syscall_idx), "D"(arg_array[0]), "S"(arg_array[1]), "d"(arg_array[2]), 
							"r"(arg3), "r"(arg4) 
						  : "rcx", "r11", "memory"); 
			break;
		}
		case 6: {
			register uint64_t arg3 asm("r10") = (uint64_t)arg_array[3]; 
			register uint64_t arg4 asm("r8")  = (uint64_t)arg_array[4]; 
			register uint64_t arg5 asm("r9")  = (uint64_t)arg_array[5]; 
			asm volatile ("syscall" 
						  : "=d"(ret.ret) 
						  : "a"(syscall_idx), "D"(arg_array[0]), "S"(arg_array[1]), "d"(arg_array[2]), 
							"r"(arg3), "r"(arg4), "r"(arg5) 
						  : "rcx", "r11", "memory"); 
			break;
		}
        default:
            return SyscallRet {-1};
    }

    return ret;	
}

}

#endif
