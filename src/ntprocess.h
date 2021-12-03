/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <functional>
#include <cstdint>

extern uint32_t readMemory64(HANDLE handle, uint64_t address, uint32_t length, void *data);
extern bool getModules(HANDLE handle, const std::function<bool(uint64_t, uint64_t, const wchar_t*)> &cb);
