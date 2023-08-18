/**
 * @file boot.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 *
 */

#include <cstdio>

#include <caprese/arch/boot_info.h>
#include <caprese/main.h>

namespace {
    caprese::arch::boot_info_t boot_info;
}

extern "C" [[noreturn]] void start(uint64_t hartid, const char* device_tree_blob) {
  using namespace caprese;
  using namespace caprese::arch;

  printf("Kernel is booting on hart %lu...\n\n", hartid);

  boot_info.hartid           = hartid;
  boot_info.device_tree_blob = device_tree_blob;

  asm volatile("mv tp, %0" : : "r"(hartid));

  main(&boot_info);
}
