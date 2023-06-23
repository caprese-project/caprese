/**
 * @file panic.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Declare a function to safely stop the kernel in the event of a fatal condition.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

#ifndef CAPRESE_PANIC_H_
#define CAPRESE_PANIC_H_

namespace caprese {
  [[noreturn]] void panic(const char* fmt, ...);
} // namespace caprese

#endif // CAPRESE_PANIC_H_
