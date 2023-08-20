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
#include <cstddef>
#include <cstdint>

#include <caprese/memory/address.h>

namespace caprese::capability {
  constexpr uint8_t CLASS_FLAG_VALID         = 1 << 0;
  constexpr uint8_t CLASS_FLAG_BUILTIN       = 1 << 1;
  constexpr uint8_t CLASS_FLAG_CONSTRUCTIBLE = 1 << 2;
  constexpr uint8_t CLASS_FLAG_MOVABLE       = 1 << 3;
  constexpr uint8_t CLASS_FLAG_COPYABLE      = 1 << 4;

  using ccid_t = uint16_t;
  static_assert(8 * sizeof(ccid_t) == CONFIG_MAX_CAPABILITY_CLASSES_BIT);

  using cap_gen_t = uint16_t;

  using cap_ref_t = uint32_t;
  static_assert(8 * sizeof(cap_ref_t) >= static_cast<size_t>(std::countr_zero<size_t>(CONFIG_MAX_CAPABILITIES)));

  constexpr size_t CAP_REF_SIZE_BIT = std::countr_zero<size_t>(CONFIG_MAX_CAPABILITIES);

  struct cap_ret_t {
    uintptr_t result;
    uintptr_t error;
  };

  union capability_t;

  using method_t = cap_ret_t (*)(capability_t*, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

  struct class_t {
    const char* name;
    uint8_t     flags;
    uint8_t     num_permissions;
    uint8_t     num_fields;
    uint8_t     num_methods;
    uint32_t    reserved;
    method_t*   methods;
  };

  static_assert(sizeof(class_t) == CONFIG_CAPABILITY_CLASS_SIZE);

  union capability_t {
    const ccid_t ccid;

    // ccid != 0
    struct {
      ccid_t                   ccid;
      cap_gen_t                cid_generation;
      uint32_t                 tid;
      memory::mapped_address_t instance;
    } info;

    // ccid == 0
    struct {
      const ccid_t    ccid;
      const cap_gen_t cid_generation;
      cap_ref_t       prev_free_list: CAP_REF_SIZE_BIT;
      cap_ref_t       next_free_list: CAP_REF_SIZE_BIT;
      uint8_t         prev_free_index;
    } management;
  };

  static_assert(sizeof(capability_t) == CONFIG_CAPABILITY_SIZE);

  [[nodiscard]] bool          init_capability_class_space();
  [[nodiscard]] bool          init_capability_space();
  [[nodiscard]] class_t*      create_capability_class();
  [[nodiscard]] size_t        instance_size(class_t* cap_class);
  [[nodiscard]] capability_t* create_capability(ccid_t ccid);
  void                        delete_capability(capability_t* capability);
  [[nodiscard]] capability_t* copy_capability(capability_t* capability, uint64_t permissions);
  [[nodiscard]] class_t*      lookup_class(ccid_t ccid);
  ccid_t                      make_ccid(class_t* cap_class);
  cap_ref_t                   make_ref(capability_t* capability);
  [[nodiscard]] cap_ret_t     call_method(capability_t* capability, uint8_t method, uintptr_t arg0 = 0, uintptr_t arg1 = 0, uintptr_t arg2 = 0, uintptr_t arg3 = 0);
  void                        set_field(capability_t* capability, uint8_t field, uintptr_t value);
  [[nodiscard]] cap_ret_t     get_field(capability_t* capability, uint8_t field);
  void                        set_permission(capability_t* capability, uint8_t permission, bool value);
  [[nodiscard]] cap_ret_t     is_permitted(capability_t* capability, uint8_t permission);
} // namespace caprese::capability

#endif // CAPRESE_CAPABILITY_H_
