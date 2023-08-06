/**
 * @file main.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declare platform-independent kernel main.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

#ifndef CAPRESE_MAIN_H_
#define CAPRESE_MAIN_H_

#include <caprese/arch/boot_info.h>

namespace caprese {
  [[noreturn]] void main(arch::boot_info_t* boot_info);
} // namespace caprese

#endif // CAPRESE_MAIN_H_
