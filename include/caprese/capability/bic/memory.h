#ifndef CAPRESE_CAPABILITY_BIC_MEMORY_H_
#define CAPRESE_CAPABILITY_BIC_MEMORY_H_

#include <cstdint>

#include <caprese/capability/capability.h>
#include <caprese/memory/address.h>

namespace caprese::capability::bic::memory {
  constexpr uint16_t CCID = 1;

  namespace permission {
    constexpr uint8_t READABLE   = 0;
    constexpr uint8_t WRITABLE   = 1;
    constexpr uint8_t EXECUTABLE = 2;

    constexpr uint64_t ALL = 0b111;
  } // namespace permission

  namespace field {
    constexpr uint8_t PHYSICAL_ADDRESS = 0;
    constexpr uint8_t VIRTUAL_ADDRESS  = 1;
    constexpr uint8_t TID              = 2;
  } // namespace field

  namespace method {
    constexpr uint8_t MAP   = 0;
    constexpr uint8_t UNMAP = 1;
    constexpr uint8_t READ  = 2;
    constexpr uint8_t WRITE = 3;
  } // namespace method

  namespace constant {
    constexpr uint8_t READABLE   = 1 << 0;
    constexpr uint8_t WRITABLE   = 1 << 1;
    constexpr uint8_t EXECUTABLE = 1 << 2;
  } // namespace constant

  class_t*      create_class();
  capability_t* create(caprese::memory::physical_address_t physical_address, uint8_t flags);

  cap_ret_t method_map(capability_t* cap, uintptr_t _task_cid_handle, uintptr_t _virtual_address, uintptr_t _flags, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_unmap(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_read(capability_t* cap, uintptr_t offset, uintptr_t size, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_write(capability_t* cap, uintptr_t value, uintptr_t offset, uintptr_t size, [[maybe_unused]] uintptr_t arg3);
} // namespace caprese::capability::bic::memory

#endif // CAPRESE_CAPABILITY_BIC_MEMORY_H_
