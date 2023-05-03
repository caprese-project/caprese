/**
 * @file types.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declares the types used throughout the kernel.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/LICENSE
 *
 */

#ifndef CAPRESE_KERNEL_TYPES_H_
#define CAPRESE_KERNEL_TYPES_H_

#include <cstdint>

namespace caprese {
  using virtual_address_t  = uintptr_t;
  using physical_address_t = uintptr_t;
} // namespace caprese

#endif // CAPRESE_KERNEL_TYPES_H_
