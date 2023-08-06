#ifndef CAPRESE_UTIL_ARRAY_SIZE_H_
#define CAPRESE_UTIL_ARRAY_SIZE_H_

#include <cstddef>

namespace caprese::inline util {
  template<typename T, size_t N>
  constexpr size_t array_size_of(const T (&)[N]) {
    return N;
  }
} // namespace caprese::inline util

#endif // CAPRESE_UTIL_ARRAY_SIZE_H_
