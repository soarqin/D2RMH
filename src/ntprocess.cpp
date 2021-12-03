/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "ntprocess.h"

#include "os_structs.h"

#include <vector>
#include <cstdint>
#include <stdexcept>

#if defined(_M_IX86)

#define IS_TRUE(clause, msg) if (!(clause)) { throw std::runtime_error(msg); }

void checkIfProcessIsX64(HANDLE handle) {
    BOOL is_wow64_process = TRUE;
    IS_TRUE(::IsWow64Process(handle, &is_wow64_process), "IsWow64Process failed");
    IS_TRUE(FALSE == is_wow64_process, "Target process is not x64 one");
}

uint32_t readMemory64(HANDLE handle, uint64_t address, uint32_t length, void *data) {
    IS_TRUE(handle, "No process handle obtained");

    NTSTATUS status = NtWow64ReadVirtualMemory64(handle, address, data, length, FALSE);

    if(!NT_SUCCESS(status)) {
        return 0;
    }
    return length;
}

void readPBI(HANDLE handle, sys::PROCESS_BASIC_INFORMATION64 &pbi) {
    IS_TRUE(handle, "No process handle obtained");

    NTSTATUS status = NtWow64QueryInformationProcess64(handle, sys::ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);

    IS_TRUE(NT_SUCCESS(status), "NtQueryInformationProcess failed");
}

uint32_t readPEBData(HANDLE handle, sys::PEB64 *peb) {
    sys::PROCESS_BASIC_INFORMATION64 pbi = {0};
    readPBI(handle, pbi);

    return readMemory64(handle, pbi.PebBaseAddress, sizeof(sys::PEB64), peb);
}

bool getModules(HANDLE handle, const std::function<bool(uint64_t, uint64_t, const wchar_t*)> &cb) {
    sys::PEB64 peb;
    readPEBData(handle, &peb);

    // ------------------------------------------------------------------------
    // Read memory from pointer to loader data structures.
    // ------------------------------------------------------------------------
    std::vector<uint8_t> read_peb_ldr_data(sizeof(sys::PEB_LDR_DATA64));
    readMemory64(handle, (uint64_t)peb.LoaderData, sizeof(sys::PEB_LDR_DATA64), read_peb_ldr_data.data());
    auto *peb_ldr_data = (sys::PEB_LDR_DATA64 *)read_peb_ldr_data.data();

    ULONGLONG address = peb_ldr_data->InLoadOrderModuleList.Flink;

    // ------------------------------------------------------------------------
    // Traversing loader data structures.
    // ------------------------------------------------------------------------
    for (;;) {
        std::vector<uint8_t> read_ldr_table_entry(sizeof(sys::LDR_DATA_TABLE_ENTRY64));
        readMemory64(handle, address, sizeof(sys::LDR_DATA_TABLE_ENTRY64), read_ldr_table_entry.data());

        auto *ldr_table_entry = (sys::LDR_DATA_TABLE_ENTRY64 *)read_ldr_table_entry.data();
        if (!ldr_table_entry->BaseAddress) {
            break;
        }

        std::vector<uint8_t> unicode_name(ldr_table_entry->BaseDllName.MaximumLength);
        readMemory64(handle,
                     ldr_table_entry->BaseDllName.Buffer,
                     ldr_table_entry->BaseDllName.MaximumLength,
                     unicode_name.data());

        if (!cb(ldr_table_entry->BaseAddress, ldr_table_entry->SizeOfImage, (const wchar_t*)unicode_name.data())) {
            break;
        }

        ldr_table_entry = (sys::LDR_DATA_TABLE_ENTRY64 *)read_ldr_table_entry.data();
        address = (uint64_t)ldr_table_entry->InLoadOrderModuleList.Flink;
    }

    return true;
}

#elif defined(_M_X64)

#include <tlhelp32.h>

uint32_t readMemory64(HANDLE handle, uint64_t address, uint32_t length, void *data) {
    SIZE_T bytesRead;
    if (!ReadProcessMemory(handle, (LPCVOID)(UINT_PTR)(address), data, length, &bytesRead)) {
        return 0;
    }
    return uint32_t(bytesRead);
}

bool getModules(HANDLE handle, const std::function<bool(uint64_t, uint64_t, const wchar_t*)> &cb) {
    auto snap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, GetProcessId(handle) );
    if (snap == INVALID_HANDLE_VALUE) {
        return false;
    }
    MODULEENTRY32W me32 = {.dwSize = sizeof(me32)};
    if(!Module32FirstW(snap, &me32)) {
        CloseHandle( snap );
        return false;
    }
    do {
        if (!cb((uint64_t)(UINT_PTR)me32.modBaseAddr, me32.modBaseSize, me32.szModule)) {
            break;
        }
    } while (Module32NextW(snap, &me32));
    CloseHandle(snap);
    return true;
}

#else

#   error "This application must be built as an x86 executable"

#endif
