#include <cstdint>

extern "C" int32_t __ctzdi2(int64_t value) {
  if (value == 0) return 64;

  int32_t count = 0;

  while ((value & 1) == 0) {
    value >>= 1;
    count++;
  }

  return count;
}
