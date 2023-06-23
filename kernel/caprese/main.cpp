/**
 * @file main.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief Implement platform-independent kernel main.
 * @since 0.0.1
 * @version 0.0.1
 *
 * @copyright (c) 2023 cosocaf
 *
 * This project is released under the MIT License.
 * @see https://github.com/cosocaf/caprese/blob/master/LICENSE
 */

#include <cstdio>

#include <caprese/main.h>
#include <caprese/panic.h>
#include <caprese/task/task.h>

constexpr auto LOGO_TEXT = R"(
  ____
 / ___|  __ _  _ __   _ __   ___  ___   ___
| |     / _` || '_ \ | '__| / _ \/ __| / _ \
| |___ | (_| || |_) || |   |  __/\__ \|  __/
 \____| \__,_|| .__/ |_|    \___||___/ \___|
              |_|

)";

namespace caprese {
  [[noreturn]] void main() {
    printf(LOGO_TEXT);
    task::switch_to(task::get_task_by_id(1));
    panic("TEST PANIC");
  }
} // namespace caprese
