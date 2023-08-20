#ifndef CAPRESE_CAPABILITY_BIC_TASK_H_
#define CAPRESE_CAPABILITY_BIC_TASK_H_

#include <cstdint>

#include <caprese/capability/capability.h>
#include <caprese/task/task.h>

namespace caprese::capability::bic::task {
  constexpr uint16_t CCID = 2;

  namespace permission {
    constexpr uint8_t SWITCHABLE = 0;
    constexpr uint8_t KILLABLE   = 1;
  } // namespace permission

  namespace field {
    constexpr uint8_t TID = 0;
  } // namespace field

  namespace method {
    constexpr uint8_t STATE        = 0;
    constexpr uint8_t SWITCH       = 1;
    constexpr uint8_t SET_REGISTER = 2;
    constexpr uint8_t ALIVE        = 3;
    constexpr uint8_t KILL         = 4;
  } // namespace method

  class_t*      create_class();
  capability_t* create(caprese::task::tid_t tid);

  cap_ret_t method_state(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_switch(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_set_register(capability_t* cap, uintptr_t reg, uintptr_t value, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_alive(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_kill(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
} // namespace caprese::capability::bic::task

#endif // CAPRESE_CAPABILITY_BIC_TASK_H_
