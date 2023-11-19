#include <cassert>
#include <utility>

#include <kernel/frame.h>
#include <log/log.h>

namespace {
  constexpr const char* tag = "kernel/frame";
} // namespace

uintptr_t set_register(map_ptr<frame_t> frame, uintptr_t reg, uintptr_t value) {
  assert(frame != nullptr);

  if (reg > LAST_REGISTER) {
    loge(tag, "Invalid register: %lu", reg);
  } else {
    std::swap(frame.as<uintptr_t>()[reg], value);
  }

  return value;
}

uintptr_t get_register(map_ptr<frame_t> frame, uintptr_t reg) {
  assert(frame != nullptr);

  if (reg > LAST_REGISTER) {
    loge(tag, "Invalid register: %lu", reg);
    return 0;
  }

  return frame.as<uintptr_t>()[reg];
}
