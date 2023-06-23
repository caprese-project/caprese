/**
 * @file system_call.h
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

#ifndef CAPRESE_ARCH_RISCV_SYSTEM_CALL_H_
#define CAPRESE_ARCH_RISCV_SYSTEM_CALL_H_

namespace caprese::arch::trap {
  void handle_system_call();
}

#endif // CAPRESE_ARCH_RISCV_SYSTEM_CALL_H_
