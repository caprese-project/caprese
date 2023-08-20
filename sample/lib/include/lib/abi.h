#ifndef LIB_ABI_H_
#define LIB_ABI_H_

#include <cstdint>

#include <caprese/arch/task.h>
#include <caprese/util/array.h>

constexpr uintptr_t ABI_RETURN_ADDRESS  = caprese::arch::ABI_RETURN_ADDRESS;
constexpr uintptr_t ABI_STACK_POINTER   = caprese::arch::ABI_STACK_POINTER;
constexpr uintptr_t ABI_GLOBAL_POINTER  = caprese::arch::ABI_GLOBAL_POINTER;
constexpr uintptr_t ABI_THREAD_POINTER  = caprese::arch::ABI_THREAD_POINTER;
constexpr uintptr_t ABI_PROGRAM_COUNTER = caprese::arch::ABI_PROGRAM_COUNTER;

#endif // LIB_ABI_H_
