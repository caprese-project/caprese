#ifndef KERNEL_ALIGN_H_
#define KERNEL_ALIGN_H_

#include <cstddef>

template<typename T>
[[nodiscard]] constexpr T round_up(T value, size_t align) {
  if (align == 0) {
    return value;
  }
  return (value + align - 1) / align * align;
}

template<typename T>
[[nodiscard]] constexpr T round_down(T value, size_t align) {
  if (align == 0) {
    return value;
  }
  return value / align * align;
}

#endif // KERNEL_ALIGN_H_
