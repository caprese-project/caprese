#ifndef CAPRESE_LIB_ENDIAN_H_
#define CAPRESE_LIB_ENDIAN_H_

#include <concepts>

namespace caprese {
  template<std::integral T>
  inline constexpr T swap_endian(T value) {
    T result = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
      result |= ((value >> (i * 8)) & 0xFF) << ((sizeof(T) - i - 1) * 8);
    }
    return result;
  }
} // namespace caprese

#endif // CAPRESE_LIB_ENDIAN_H_
