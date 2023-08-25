#include "arch/syscall.inc"

.section .text

DEFINE_SYSCALL(base, null, 0, 0)
DEFINE_SYSCALL(base, core_id, 0, 1)
DEFINE_SYSCALL(base, page_size, 0, 2)
DEFINE_SYSCALL(base, user_space_start, 0, 3)
DEFINE_SYSCALL(base, user_space_end, 0, 4)

DEFINE_SYSCALL(debug, putchar, 1, 0)
DEFINE_SYSCALL(debug, getchar, 1, 1)

DEFINE_SYSCALL(cap, call_method, 2, 2)
DEFINE_SYSCALL(cap, get_field, 2, 3)
DEFINE_SYSCALL(cap, is_permitted, 2, 4)
DEFINE_SYSCALL(cap, list_base, 2, 5)
DEFINE_SYSCALL(cap, list_size, 2, 6)
DEFINE_SYSCALL(cap, move, 2, 7)
DEFINE_SYSCALL(cap, copy, 2, 8)

DEFINE_SYSCALL(task, tid, 3, 0)
DEFINE_SYSCALL(task, create, 3, 1)
DEFINE_SYSCALL(task, yield, 3, 2)
DEFINE_SYSCALL(task, ipc_receive, 3, 3)
