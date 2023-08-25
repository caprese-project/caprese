#include <cstdarg>
#include <cstdio>

#include <caprese/util/printf.h>

#include <console/ipc.h>
#include <runtime/memory.h>
#include <runtime/syscall.h>
#include <runtime/task.h>

extern "C" {
  extern task_handle_t __console_task_handle;
  extern size_t        __page_size;

  memory_handle_t __console_buffer_handle;
  char*           __console_buffer;
  size_t          __console_buffer_seek_position;

  bool __init_console() {
    __console_buffer_handle = fetch_memory_handle(MEMORY_FLAG_READABLE | MEMORY_FLAG_WRITABLE);
    if (__console_buffer_handle == 0) [[unlikely]] {
      return false;
    }

    __console_buffer = reinterpret_cast<char*>(get_virtual_address(__console_buffer_handle));
    if (__console_buffer == 0) [[unlikely]] {
      return false;
    }

    __console_buffer_seek_position = 0;

    return true;
  }

  void __flush_console() {
    if (__console_task_handle == 0) [[unlikely]] {
      // fallback
      for (size_t i = 0; i < __console_buffer_seek_position; ++i) {
        sys_debug_putchar(__console_buffer[i]);
      }
      __console_buffer_seek_position = 0;
      return;
    }

    bool result = unmap_memory_page(__console_buffer_handle);
    if (!result) [[unlikely]] {
      return;
    }

    ipc_msg_t msg {
      .flags   = IPC_MSG_FLAG_TYPE_MSG,
      .channel = CONSOLE_IPC_CHANNEL_WRITE,
      .msg = {
        .handles = { __console_buffer_handle },
      },
    };
    ipc_send(__console_task_handle, &msg);

    __init_console();
  }

  void __put_console_buffer(char ch) {
    if (__console_buffer == nullptr) [[unlikely]] {
      return;
    }
    __console_buffer[__console_buffer_seek_position] = ch;
    ++__console_buffer_seek_position;
    if (ch == '\n' || __console_buffer_seek_position >= __page_size) [[unlikely]] {
      __flush_console();
    }
  }

  int printf(const char* fmt, ...) {
    va_list arg;
    int     len;
    va_start(arg, fmt);
    len = vprintf(fmt, arg);
    va_end(arg);
    return len;
  }

  int vprintf(const char* fmt, va_list arg) {
    size_t     size     = 0;
    const auto callback = [&size](char ch) {
      __put_console_buffer(ch);
      ++size;
    };
    caprese::printf_template::printf(fmt, arg, callback);
    return size;
  }

  int putchar(int ch) {
    __put_console_buffer(ch);
    return ch;
  }

  int puts(const char* str) {
    while (*str) {
      __put_console_buffer(*str);
      ++str;
    }
    __put_console_buffer('\n');
    return 0;
  }
}
