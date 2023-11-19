#ifndef KERNEL_START_H_
#define KERNEL_START_H_

#include <kernel/boot.h>

[[noreturn]] void start(map_ptr<boot_info_t> boot_info);

#endif // KERNEL_START_H_
