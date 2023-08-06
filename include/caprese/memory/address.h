/**
 * @file address.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#ifndef CAPRESE_MEMORY_ADDRESS_H_
#define CAPRESE_MEMORY_ADDRESS_H_

#include <cassert>
#include <cstdint>

#include <caprese/arch/common/memory/layout.h>

namespace caprese::memory {
  class address_t {
    uintptr_t address;

  public:
    constexpr address_t(uintptr_t address): address(address) { }

    constexpr address_t(void* address): address(reinterpret_cast<uintptr_t>(address)) { }

    constexpr uintptr_t value() {
      return address;
    }

    template<typename T>
    constexpr T as() {
      return reinterpret_cast<T>(reinterpret_cast<void*>(address));
    }
  };

  using virtual_address_t  = address_t;
  using physical_address_t = address_t;

  constexpr virtual_address_t phys_to_virt(physical_address_t phys_addr) {
    auto address = arch::memory::begin_of_phys_map_space + phys_addr.value();
    assert(arch::memory::begin_of_phys_map_space <= address || address <= arch::memory::end_of_phys_map_space);
    return address;
  }

  constexpr physical_address_t virt_to_phys(virtual_address_t virt_addr) {
    assert(arch::memory::begin_of_phys_map_space <= virt_addr.value() || virt_addr.value() <= arch::memory::end_of_phys_map_space);
    return virt_addr.value() - arch::memory::begin_of_phys_map_space;
  }
} // namespace caprese::memory

#endif // CAPRESE_MEMORY_ADDRESS_H_
