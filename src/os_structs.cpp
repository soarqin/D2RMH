/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "os_structs.h"

extern "C" {
static HMODULE ntdllMod = LoadLibraryA("ntdll.dll");
NtWow64QueryInformationProcess64Proc NtWow64QueryInformationProcess64
    = (NtWow64QueryInformationProcess64Proc)GetProcAddress(ntdllMod, "NtWow64QueryInformationProcess64");
NtWow64ReadVirtualMemory64Proc NtWow64ReadVirtualMemory64
    = (NtWow64ReadVirtualMemory64Proc)GetProcAddress(ntdllMod, "NtWow64ReadVirtualMemory64");
}
