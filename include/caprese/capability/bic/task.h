#ifndef CAPRESE_CAPABILITY_BIC_TASK_H_
#define CAPRESE_CAPABILITY_BIC_TASK_H_

#include <cstdint>

#include <caprese/capability/capability.h>
#include <caprese/task/types.h>

namespace caprese::capability::bic::task {
  constexpr uint16_t CCID = 2;

  namespace permission {
    constexpr uint8_t SWITCHABLE = 0;
    constexpr uint8_t KILLABLE   = 1;
    constexpr uint8_t SENDABLE   = 2;

    constexpr uint64_t ALL = 0b111;
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
    constexpr uint8_t SEND         = 5;
    constexpr uint8_t NB_SEND      = 6;
  } // namespace method

  namespace constant {
    constexpr uint8_t STATE_UNUSED   = caprese::task::TASK_STATE_UNUSED;
    constexpr uint8_t STATE_CREATING = caprese::task::TASK_STATE_CREATING;
    constexpr uint8_t STATE_RUNNING  = caprese::task::TASK_STATE_RUNNING;
    constexpr uint8_t STATE_READY    = caprese::task::TASK_STATE_READY;
  } // namespace constant

  class_t*      create_class();
  capability_t* create(caprese::task::tid_t tid);

  cap_ret_t method_state(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_switch(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_set_register(capability_t* cap, uintptr_t reg, uintptr_t value, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_alive(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_kill(capability_t* cap, [[maybe_unused]] uintptr_t arg0, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_send(capability_t* cap, uintptr_t msg, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
  cap_ret_t method_nb_send(capability_t* cap, uintptr_t msg, [[maybe_unused]] uintptr_t arg1, [[maybe_unused]] uintptr_t arg2, [[maybe_unused]] uintptr_t arg3);
} // namespace caprese::capability::bic::task

#endif // CAPRESE_CAPABILITY_BIC_TASK_H_
