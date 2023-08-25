#ifndef RUNTIME_TYPES_H_
#define RUNTIME_TYPES_H_

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  typedef uint32_t handle_t;
  typedef handle_t memory_handle_t;
  typedef handle_t task_handle_t;

  typedef uint32_t tid_t;

  typedef uint8_t task_state_t;

  typedef struct _tag_sysret {
    uintptr_t result;
    uintptr_t error;
  } sysret_t;

#define IPC_MSG_FLAG_TYPE_SHORT_MSG 1
#define IPC_MSG_FLAG_TYPE_MSG       2
#define IPC_MSG_FLAG_TYPE_LONG_MSG  3
#define IPC_MSG_FLAG_TYPE_MASK      0b11
#define IPC_MSG_FLAG_CANCELED       4

  typedef struct _tag_ipc_msg {
    uint32_t flags  : 4;
    uint32_t channel: 28;

    union {
      struct {
        uintptr_t data[4];
      } short_msg;

      struct {
        memory_handle_t handles[4];
      } msg;

      struct {
        size_t          msg_size;
        memory_handle_t table_handle;
      } long_msg;
    };
  } ipc_msg_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RUNTIME_TYPES_H_
