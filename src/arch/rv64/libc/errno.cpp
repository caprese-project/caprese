#include <kernel/cls.h>

extern "C" {
  int* __errno() {
    return &get_cls()->errno_value;
  }
}
