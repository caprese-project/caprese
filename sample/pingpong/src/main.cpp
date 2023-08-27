#include <cstdio>

#include <runtime/types.h>
#include <runtime/task.h>

void nop() {}

extern "C" int main(task_handle_t pingpong) {
  tid_t tid = get_tid();
  printf("Hello, pingpong!\n");
  while (true) {
    printf("ping-pong! tid: %u\n", tid);
    for (int i = 0; i < 10'000'000; ++i) {
      nop();
    }
    switch_task(pingpong);
  }
}
