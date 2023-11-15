#ifndef KERNEL_ADDRESS_H_
#define KERNEL_ADDRESS_H_

#include <compare>
#include <cstddef>
#include <cstdint>
#include <type_traits>

template<typename Derived>
struct _addr_t {
  uintptr_t value;

  [[nodiscard]] constexpr bool is_aligned_to_pow2(uintptr_t n) {
    return (value & (n - 1)) == 0;
  }

  template<typename T>
  [[nodiscard]] constexpr T as() const {
    return reinterpret_cast<T>(value);
  }

  [[nodiscard]] constexpr static Derived from(uintptr_t addr) {
    return Derived { addr };
  }

  [[nodiscard]] constexpr static Derived from(const void* ptr) {
    return Derived { reinterpret_cast<uintptr_t>(ptr) };
  }

  [[nodiscard]] constexpr static Derived from([[maybe_unused]] nullptr_t) {
    return Derived { 0 };
  }

  [[nodiscard]] auto operator<=>(const _addr_t& addr) const = default;

  [[nodiscard]] constexpr bool operator==(nullptr_t) const {
    return value == 0;
  }

  [[nodiscard]] constexpr bool operator!=(nullptr_t) const {
    return value != 0;
  }

  [[nodiscard]] constexpr Derived operator+(size_t n) const {
    return Derived::from(value + n);
  }

  [[nodiscard]] constexpr Derived operator+(ptrdiff_t n) const {
    return Derived::from(value + n);
  }
};

struct map_addr_t;

struct phys_addr_t: public _addr_t<phys_addr_t> {
  constexpr map_addr_t as_map();

  constexpr static phys_addr_t from_map(map_addr_t map_addr);
};

static_assert(std::is_trivial_v<phys_addr_t>);
static_assert(std::is_standard_layout_v<phys_addr_t>);
static_assert(sizeof(phys_addr_t) == sizeof(uintptr_t));

struct virt_addr_t: public _addr_t<virt_addr_t> { };

static_assert(std::is_trivial_v<virt_addr_t>);
static_assert(std::is_standard_layout_v<virt_addr_t>);
static_assert(sizeof(virt_addr_t) == sizeof(uintptr_t));

struct map_addr_t: public _addr_t<map_addr_t> {
  constexpr phys_addr_t as_phys();

  constexpr static map_addr_t from_phys(phys_addr_t phys_addr);
};

static_assert(std::is_trivial_v<map_addr_t>);
static_assert(std::is_standard_layout_v<map_addr_t>);
static_assert(sizeof(map_addr_t) == sizeof(uintptr_t));

constexpr map_addr_t phys_addr_t::as_map() {
  return map_addr_t::from(value + CONFIG_MAPPED_SPACE_BASE);
}

constexpr phys_addr_t phys_addr_t::from_map(map_addr_t map_addr) {
  return phys_addr_t::from(map_addr.value - CONFIG_MAPPED_SPACE_BASE);
}

constexpr phys_addr_t map_addr_t::as_phys() {
  return phys_addr_t::from(value - CONFIG_MAPPED_SPACE_BASE);
}

constexpr map_addr_t map_addr_t::from_phys(phys_addr_t phys_addr) {
  return map_addr_t::from(phys_addr.value + CONFIG_MAPPED_SPACE_BASE);
}

#endif // KERNEL_ADDRESS_H_
