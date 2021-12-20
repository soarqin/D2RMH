/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace util {

/* String functions */
std::wstring utf8toucs4(const std::string &s);
std::vector<std::string> splitString(const std::string &str, char c);

/* Time series functions */
void updateTime();
std::chrono::steady_clock::time_point getCurrTime();

/* Hotkey functions */
uint32_t mapStringToVKey(const std::string &name, uint32_t &mods);

}
