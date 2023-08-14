#ifndef CAPRESE_UTIL_CONCEPTS_H_
#define CAPRESE_UTIL_CONCEPTS_H_

#include <type_traits>

namespace caprese::inline util {
  template<typename T>
  concept function_pointer = std::is_function_v<std::remove_pointer_t<T>>;
} // namespace caprese::inline util

#endif // CAPRESE_UTIL_CONCEPTS_H_
