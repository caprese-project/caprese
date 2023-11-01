#ifndef KERNEL_ARRAY_H_
#define KERNEL_ARRAY_H_

#include <cstddef>
#include <type_traits>

template<typename T, size_t N>
[[nodiscard]] constexpr size_t array_size_of(const T (&)[N]) {
  return N;
}

template<typename T, typename M>
[[nodiscard]] constexpr size_t array_size_of(M T::*member_reference) {
  using Elem = typename std::remove_reference_t<decltype((static_cast<T*>(nullptr)->*member_reference)[0])>;
  return sizeof(M) / sizeof(Elem);
}

#endif // KERNEL_ARRAY_H_
