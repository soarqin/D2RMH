/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "os_structs.h"

extern "C" {

NtWow64QueryInformationProcess64Proc NtWow64QueryInformationProcess64;
NtWow64ReadVirtualMemory64Proc NtWow64ReadVirtualMemory64;

static HMODULE getNtDllMod() {
    static HMODULE ntdllMod = LoadLibraryA("ntdll.dll");
    return ntdllMod;
}

void osInit() {
    NtWow64QueryInformationProcess64 =
        (NtWow64QueryInformationProcess64Proc)GetProcAddress(getNtDllMod(), "NtWow64QueryInformationProcess64");
    NtWow64ReadVirtualMemory64 =
        (NtWow64ReadVirtualMemory64Proc)GetProcAddress(getNtDllMod(), "NtWow64ReadVirtualMemory64");
}

}
