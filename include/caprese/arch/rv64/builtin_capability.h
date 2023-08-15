#ifndef CAPRESE_ARCH_RV64_BUILTIN_CAPABILITY_H_
#define CAPRESE_ARCH_RV64_BUILTIN_CAPABILITY_H_

#include <cstdint>

namespace caprese::arch::inline rv64 {
  constexpr uint16_t NULL_CAP_CCID   = 0;
  constexpr uint16_t MEMORY_CAP_CCID = 1;
  constexpr uint16_t TASK_CAP_CCID   = 2;
  constexpr uint16_t TRAP_CAP_CCID   = 3;

  constexpr uint8_t MEMORY_CAP_PERMISSION_READABLE   = 0;
  constexpr uint8_t MEMORY_CAP_PERMISSION_WRITABLE   = 1;
  constexpr uint8_t MEMORY_CAP_PERMISSION_EXECUTABLE = 2;
  constexpr uint8_t MEMORY_CAP_PERMISSION_COPYABLE   = 3;

  constexpr uint8_t MEMORY_CAP_FIELD_PHYSICAL_ADDRESS = 0;
  constexpr uint8_t MEMORY_CAP_FIELD_VIRTUAL_ADDRESS  = 1;
  constexpr uint8_t MEMORY_CAP_FIELD_TID              = 2;

  constexpr uint8_t MEMORY_CAP_METHOD_MOVE  = 0;
  constexpr uint8_t MEMORY_CAP_METHOD_COPY  = 1;
  constexpr uint8_t MEMORY_CAP_METHOD_MAP   = 2;
  constexpr uint8_t MEMORY_CAP_METHOD_UNMAP = 3;
  constexpr uint8_t MEMORY_CAP_METHOD_READ  = 4;
  constexpr uint8_t MEMORY_CAP_METHOD_WRITE = 5;

  constexpr uint8_t MEMORY_CAP_MAP_FLAG_READABLE   = 1 << 0;
  constexpr uint8_t MEMORY_CAP_MAP_FLAG_WRITABLE   = 1 << 1;
  constexpr uint8_t MEMORY_CAP_MAP_FLAG_EXECUTABLE = 1 << 2;

  constexpr uint8_t TASK_CAP_FIELD_TID = 0;

  [[nodiscard]] bool create_builtin_capability_classes();
} // namespace caprese::arch::inline rv64

#endif // CAPRESE_ARCH_RV64_BUILTIN_CAPABILITY_H_
