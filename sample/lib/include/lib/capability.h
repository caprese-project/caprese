#ifndef LIB_CAPABILITY_H_
#define LIB_CAPABILITY_H_

#include <cstddef>
#include <cstdint>
#include <utility>

#include <caprese/capability/bic/memory.h>
#include <caprese/capability/bic/task.h>

using cap_handle_t = uint32_t;

constexpr uint16_t CAP_TYPE_NULL   = 0;
constexpr uint16_t CAP_TYPE_MEMORY = caprese::capability::bic::memory::CCID;
constexpr uint16_t CAP_TYPE_TASK   = caprese::capability::bic::task::CCID;

constexpr uint8_t MEMORY_CAP_PERMISSION_READABLE   = caprese::capability::bic::memory::permission::READABLE;
constexpr uint8_t MEMORY_CAP_PERMISSION_WRITABLE   = caprese::capability::bic::memory::permission::WRITABLE;
constexpr uint8_t MEMORY_CAP_PERMISSION_EXECUTABLE = caprese::capability::bic::memory::permission::EXECUTABLE;

constexpr uint8_t MEMORY_CAP_FIELD_PHYSICAL_ADDRESS = caprese::capability::bic::memory::field::PHYSICAL_ADDRESS;
constexpr uint8_t MEMORY_CAP_FIELD_VIRTUAL_ADDRESS  = caprese::capability::bic::memory::field::VIRTUAL_ADDRESS;
constexpr uint8_t MEMORY_CAP_FIELD_TID              = caprese::capability::bic::memory::field::TID;

constexpr uint8_t MEMORY_CAP_METHOD_MAP   = caprese::capability::bic::memory::method::MAP;
constexpr uint8_t MEMORY_CAP_METHOD_UNMAP = caprese::capability::bic::memory::method::UNMAP;
constexpr uint8_t MEMORY_CAP_METHOD_READ  = caprese::capability::bic::memory::method::READ;
constexpr uint8_t MEMORY_CAP_METHOD_WRITE = caprese::capability::bic::memory::method::WRITE;

constexpr uint8_t MEMORY_CAP_CONSTANT_READABLE   = caprese::capability::bic::memory::constant::READABLE;
constexpr uint8_t MEMORY_CAP_CONSTANT_WRITABLE   = caprese::capability::bic::memory::constant::WRITABLE;
constexpr uint8_t MEMORY_CAP_CONSTANT_EXECUTABLE = caprese::capability::bic::memory::constant::EXECUTABLE;

constexpr uint8_t TASK_CAP_PERMISSION_SWITCHABLE = caprese::capability::bic::task::permission::SWITCHABLE;
constexpr uint8_t TASK_CAP_PERMISSION_KILLABLE   = caprese::capability::bic::task::permission::KILLABLE;

constexpr uint8_t TASK_CAP_FIELD_TID = caprese::capability::bic::task::field::TID;

constexpr uint8_t TASK_CAP_METHOD_STATE        = caprese::capability::bic::task::method::STATE;
constexpr uint8_t TASK_CAP_METHOD_SWITCH       = caprese::capability::bic::task::method::SWITCH;
constexpr uint8_t TASK_CAP_METHOD_SET_REGISTER = caprese::capability::bic::task::method::SET_REGISTER;
constexpr uint8_t TASK_CAP_METHOD_ALIVE        = caprese::capability::bic::task::method::ALIVE;
constexpr uint8_t TASK_CAP_METHOD_KILL         = caprese::capability::bic::task::method::KILL;

uint16_t get_cap_type(cap_handle_t cap_handle);

#endif // LIB_CAPABILITY_H_
