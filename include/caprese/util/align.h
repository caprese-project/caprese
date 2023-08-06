#ifndef CAPRESE_UTIL_ALIGN_H_
#define CAPRESE_UTIL_ALIGN_H_

#include <cstddef>

namespace caprese::inline util {
  template<typename T>
  constexpr T round_up(T value, size_t align) {
    return (value + align - 1) / align * align;
  }

  template<typename T>
  constexpr T round_down(T value, size_t align) {
    return value / align * align;
  }
} // namespace caprese::inline util

#endif // CAPRESE_UTIL_ALIGN_H_
