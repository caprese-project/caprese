/**
 * @file arch_main.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief C entry point
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 */

#include <caprese/kernel/main.h>

extern "C" [[noreturn]] void arch_main(uintptr_t hartid, const void* dtb) {
  caprese::main(hartid, dtb);
}
