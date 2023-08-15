/**
 * @file capability.h
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

#ifndef CAPRESE_CAPABILITY_CAPABILITY_H_
#define CAPRESE_CAPABILITY_CAPABILITY_H_

#include <bit>
#include <cstdint>

#include <caprese/memory/address.h>

namespace caprese::capability {
  constexpr uint8_t CLASS_FLAG_VALID         = 1 << 0;
  constexpr uint8_t CLASS_FLAG_BUILTIN       = 1 << 1;
  constexpr uint8_t CLASS_FLAG_CONSTRUCTABLE = 1 << 2;
  constexpr uint8_t CLASS_FLAG_MOVABLE       = 1 << 3;
  constexpr uint8_t CLASS_FLAG_COPYABLE      = 1 << 4;

  using ccid_t = uint16_t;
  static_assert(8 * sizeof(ccid_t) == CONFIG_MAX_CAPABILITY_CLASSES_BIT);

  struct cid_t {
    uint32_t index: std::countr_zero<uintptr_t>(CONFIG_MAX_CAPABILITIES);
    uint32_t generation: 32 - std::countr_zero<uintptr_t>(CONFIG_MAX_CAPABILITIES);
  };

  static_assert(sizeof(cid_t) == sizeof(uint32_t));

  struct capret_t {
    uintptr_t result;
    uintptr_t error;
  };

  struct capability_t;

  using method_t = capret_t (*)(capability_t*, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

  struct class_t {
    const char* name;
    uint8_t     flags;
    uint8_t     num_permissions;
    uint8_t     num_fields;
    uint8_t     num_methods;
    uint8_t     idx_construct;
    uint8_t     idx_move;
    uint8_t     idx_copy;
    uint8_t     reserved;
    method_t*   methods;
  };

  static_assert(sizeof(class_t) == CONFIG_CAPABILITY_CLASS_SIZE);

  struct capability_t {
    uint32_t                 ccid: CONFIG_MAX_CAPABILITY_CLASSES_BIT;
    uint32_t                 cid_generation: 32 - std::countr_zero<uintptr_t>(CONFIG_MAX_CAPABILITIES);
    uint32_t                 tid;
    memory::mapped_address_t instance;
  };

  static_assert(sizeof(capability_t) == CONFIG_CAPABILITY_SIZE);

  [[nodiscard]] class_t*      create_capability_class();
  [[nodiscard]] capability_t* create_capability(ccid_t ccid);
  [[nodiscard]] class_t*      lookup_class(ccid_t ccid);
  [[nodiscard]] capability_t* lookup(cid_t cid);
  [[nodiscard]] capret_t      call_method(capability_t* capability, uint8_t method, uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
  void                        set_field(capability_t* capability, uint8_t field, uintptr_t value);
  [[nodiscard]] capret_t      get_field(capability_t* capability, uint8_t field);
  [[nodiscard]] capret_t      is_permitted(capability_t* capability, uint8_t permission);
} // namespace caprese::capability

#endif // CAPRESE_CAPABILITY_H_
