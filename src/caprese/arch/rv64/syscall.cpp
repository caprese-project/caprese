#include <caprese/arch/rv64/syscall.h>
#include <caprese/util/array_size.h>

namespace caprese::arch::inline rv64 {
  uintptr_t handle_system_call([[maybe_unused]] uintptr_t code,
                               [[maybe_unused]] uintptr_t arg0,
                               [[maybe_unused]] uintptr_t arg1,
                               [[maybe_unused]] uintptr_t arg2,
                               [[maybe_unused]] uintptr_t arg3,
                               [[maybe_unused]] uintptr_t arg4,
                               [[maybe_unused]] uintptr_t arg5) {
    return -1;
  }
} // namespace caprese::arch::inline rv64
