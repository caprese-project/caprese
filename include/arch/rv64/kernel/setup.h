#ifndef ARCH_RV64_KERNEL_SETUP_H_
#define ARCH_RV64_KERNEL_SETUP_H_

#include <kernel/attribute.h>
#include <kernel/boot_info.h>
#include <kernel/cap_space.h>
#include <kernel/task.h>

__init_code void setup_memory_capabilities(map_ptr<boot_info_t> boot_info);

__init_code void setup_arch_root_boot_info(map_ptr<boot_info_t> boot_info);

__init_code void* bake_stack(map_ptr<void> stack, map_ptr<void> data, size_t size);

#endif // ARCH_RV64_KERNEL_SETUP_H_
