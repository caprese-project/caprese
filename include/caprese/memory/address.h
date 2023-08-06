/**
 * @file address.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Define types for various addresses.
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

#include <cstdint>
#include <type_traits>

namespace caprese::memory {
  /**
   * @brief Base type for various address types.
   * @tparam Derived The type inherits this structure. CRTP Idioms.
   */
  template<typename Derived>
  struct base_address_t {
    /**
     * @brief The address that the instance is pointing to.
     *
     */
    uintptr_t value;

    /**
     * @brief Cast to a pointer of the specified type.
     *
     * @tparam T The type to cast.
     * @return constexpr T*
     */
    template<typename T>
    constexpr T* as() {
      return reinterpret_cast<T*>(value);
    }

    /**
     * @brief Cast to a pointer of the specified type.
     *
     * @tparam T The type to cast.
     * @return constexpr const T*
     */
    template<typename T>
    constexpr const T* as() const {
      return reinterpret_cast<const T*>(value);
    }

    /**
     * @brief Create an instance from a specified address.
     *
     * @param address The address to be specified.
     * @return constexpr Derived
     */
    constexpr static Derived from(uintptr_t address) {
      return Derived { address };
    }

    /**
     * @brief Creates an instance from a specified pointer.
     *
     * @tparam T
     * @param ptr The pointer to be specified.
     * @return constexpr Derived
     */
    template<typename T>
    constexpr static Derived from(const T* ptr) {
      return Derived { reinterpret_cast<uintptr_t>(ptr) };
    }

    /**
     * @brief Generate a null address.
     *
     * @return constexpr Derived
     */
    constexpr static Derived null() {
      return Derived { 0 };
    }
  };

  /**
   * @brief Type representing a physical address.
   *
   */
  struct physical_address_t: public base_address_t<physical_address_t> { };

  /**
   * @brief Type representing a virtual address.
   *
   */
  struct virtual_address_t: public base_address_t<virtual_address_t> { };

  /**
   * @brief Type representing a virtual address that maps one-to-one to a physical address.
   *
   */
  struct mapped_address_t: public base_address_t<mapped_address_t> {
    constexpr physical_address_t physical_address() {
      return physical_address_t::from(value - CONFIG_MAPPED_SPACE_BASE);
    }
  };

  static_assert(std::is_trivially_copyable_v<physical_address_t>);
  static_assert(std::is_standard_layout_v<physical_address_t>);
  static_assert(sizeof(physical_address_t) == sizeof(uintptr_t));
  static_assert(std::is_trivially_copyable_v<virtual_address_t>);
  static_assert(std::is_standard_layout_v<virtual_address_t>);
  static_assert(sizeof(virtual_address_t) == sizeof(uintptr_t));
  static_assert(std::is_trivially_copyable_v<mapped_address_t>);
  static_assert(std::is_standard_layout_v<mapped_address_t>);
  static_assert(sizeof(mapped_address_t) == sizeof(uintptr_t));
} // namespace caprese::memory

#endif // CAPRESE_MEMORY_ADDRESS_H_
