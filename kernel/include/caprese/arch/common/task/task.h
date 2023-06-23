/**
 * @file task.h
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

#ifndef CAPRESE_ARCH_COMMON_TASK_TASK_H_
#define CAPRESE_ARCH_COMMON_TASK_TASK_H_

#ifdef CONFIG_ARCH_RISCV64
#include <caprese/arch/rv64/task/task.h>
#else
#error "Unknown Arch"
#endif

#endif // CAPRESE_ARCH_COMMON_TASK_TASK_H_
