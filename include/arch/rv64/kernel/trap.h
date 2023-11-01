#ifndef ARCH_RV64_KERNEL_TRAP_H_
#define ARCH_RV64_KERNEL_TRAP_H_

[[noreturn]] void return_to_user_mode();

[[noreturn]] void default_trap_handler();

void set_trap_handler(void (*handler)());

void enable_trap();
void disable_trap();

#endif // ARCH_RV64_KERNEL_TRAP_H_
