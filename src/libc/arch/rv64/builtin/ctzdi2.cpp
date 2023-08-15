#include <cstdint>

extern "C" int32_t __ctzdi2(int64_t value) {
  if (value == 0) return 64;
  return __builtin_ctzll(value);
}