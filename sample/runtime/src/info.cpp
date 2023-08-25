#include <runtime/global.h>
#include <runtime/info.h>

extern "C" {
  size_t page_size() {
    return __page_size;
  }

  uintptr_t user_space_start() {
    return __user_space_start;
  }

  uintptr_t user_space_end() {
    return __user_space_end;
  }
}
