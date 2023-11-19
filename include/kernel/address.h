#ifndef KERNEL_ADDRESS_H_
#define KERNEL_ADDRESS_H_

#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

template<typename T>
struct phys_ptr;

template<typename T>
struct virt_ptr;

template<typename T>
struct map_ptr;

template<typename T, template<typename> typename Derived>
struct base_ptr {
  uintptr_t _ptr;

  constexpr T* get() const {
    return reinterpret_cast<T*>(_ptr);
  }

  constexpr uintptr_t raw() const {
    return _ptr;
  }

  template<typename U>
  constexpr Derived<U> as() const {
    return Derived<U>::from(_ptr);
  }

  constexpr T* operator->() const {
    return get();
  }

  constexpr std::add_lvalue_reference_t<T> operator*() const {
    return *get();
  }

  constexpr auto operator<=>(const base_ptr& other) const = default;

  constexpr bool operator==(const base_ptr& other) const {
    return _ptr == other._ptr;
  }

  constexpr bool operator!=(const base_ptr& other) const {
    return _ptr != other._ptr;
  }

  constexpr bool operator==(nullptr_t) const {
    return _ptr == 0;
  }

  constexpr bool operator!=(nullptr_t) const {
    return _ptr != 0;
  }

  template<std::integral U>
  constexpr Derived<T> operator+(U offset) const {
    return Derived<T>::from(_ptr + sizeof(T) * offset);
  }

  template<std::integral U>
  constexpr Derived<T> operator-(U offset) const {
    return Derived<T>::from(_ptr - sizeof(T) * offset);
  }

  template<std::integral U>
  constexpr base_ptr& operator+=(U offset) {
    _ptr += sizeof(T) * offset;
    return *this;
  }

  template<std::integral U>
  constexpr base_ptr& operator-=(U offset) {
    _ptr -= sizeof(T) * offset;
    return *this;
  }

  template<typename... Args>
  constexpr auto operator()(Args&&... args) -> decltype((*get())(std::forward<Args>(args)...)) {
    return get()(std::forward<Args>(args)...);
  }

  template<typename U = T>
  constexpr U& operator[](size_t index) const {
    static_assert(!std::is_same_v<U, void>, "Cannot use operator[] with void type.");
    return get()[index];
  }

  constexpr static Derived<T> from(nullptr_t) {
    return Derived<T> { 0 };
  }

  constexpr static Derived<T> from(uintptr_t ptr) {
    return Derived<T> { ptr };
  }

  constexpr static Derived<T> from(const void* ptr) {
    return Derived<T> { reinterpret_cast<uintptr_t>(ptr) };
  }
};

template<typename T>
struct phys_ptr: public base_ptr<T, phys_ptr> {
  template<typename U = T>
  constexpr map_ptr<U> as_map();

  template<typename U = T>
  constexpr operator map_ptr<U>() {
    return as_map();
  }
};

template<typename T>
struct virt_ptr: public base_ptr<T, virt_ptr> { };

template<typename T>
struct map_ptr: public base_ptr<T, map_ptr> {
  template<typename U = T>
  constexpr phys_ptr<U> as_phys();

  template<typename U = T>
  constexpr operator phys_ptr<U>() {
    return as_phys();
  }
};

template<typename T>
template<typename U>
constexpr map_ptr<U> phys_ptr<T>::as_map() {
  return map_ptr<U>::from(this->_ptr + CONFIG_MAPPED_SPACE_BASE);
}

template<typename T>
template<typename U>
constexpr phys_ptr<U> map_ptr<T>::as_phys() {
  return phys_ptr<U>::from(this->_ptr - CONFIG_MAPPED_SPACE_BASE);
}

static_assert(std::is_trivial_v<phys_ptr<void>>);
static_assert(std::is_trivial_v<virt_ptr<void>>);
static_assert(std::is_trivial_v<map_ptr<void>>);

struct phys_ptr_helper {
  uintptr_t ptr;

  template<typename T>
  constexpr operator phys_ptr<T>() {
    return phys_ptr<T>::from(ptr);
  }

  template<typename T>
  constexpr operator map_ptr<T>() {
    return phys_ptr<T>::from(ptr).as_map();
  }
};

struct virt_ptr_helper {
  uintptr_t ptr;

  template<typename T>
  constexpr operator virt_ptr<T>() {
    return virt_ptr<T>::from(ptr);
  }
};

struct map_ptr_helper {
  uintptr_t ptr;

  template<typename T>
  constexpr operator map_ptr<T>() {
    return map_ptr<T>::from(ptr);
  }

  template<typename T>
  constexpr operator phys_ptr<T>() {
    return map_ptr<T>::from(ptr).as_phys();
  }
};

template<typename T>
constexpr phys_ptr_helper make_phys_ptr(T* ptr) {
  return phys_ptr_helper { reinterpret_cast<uintptr_t>(ptr) };
}

template<std::integral T>
constexpr phys_ptr_helper make_phys_ptr(T ptr) {
  return phys_ptr_helper { static_cast<uintptr_t>(ptr) };
}

template<typename T>
constexpr virt_ptr_helper make_virt_ptr(T* ptr) {
  return virt_ptr_helper { reinterpret_cast<uintptr_t>(ptr) };
}

template<std::integral T>
constexpr virt_ptr_helper make_virt_ptr(T ptr) {
  return virt_ptr_helper { static_cast<uintptr_t>(ptr) };
}

template<typename T>
constexpr map_ptr_helper make_map_ptr(T* ptr) {
  return map_ptr_helper { reinterpret_cast<uintptr_t>(ptr) };
}

template<std::integral T>
constexpr map_ptr_helper make_map_ptr(T ptr) {
  return map_ptr_helper { static_cast<uintptr_t>(ptr) };
}

constexpr phys_ptr_helper operator""_phys(unsigned long long int ptr) {
  return phys_ptr_helper { static_cast<uintptr_t>(ptr) };
}

constexpr virt_ptr_helper operator""_virt(unsigned long long int ptr) {
  return virt_ptr_helper { static_cast<uintptr_t>(ptr) };
}

constexpr map_ptr_helper operator""_map(unsigned long long int ptr) {
  return map_ptr_helper { static_cast<uintptr_t>(ptr) };
}

#endif // KERNEL_ADDRESS_H_
