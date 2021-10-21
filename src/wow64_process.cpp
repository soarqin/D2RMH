/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "wow64_process.h"

#include "os_structs.h"

#include <vector>
#include <cstdint>
#include <stdexcept>

#ifndef WIN32
#   error "This application must be built as an x86 executable"
#endif

#define GET_FUNC_ADDR(name) _##name name = (_##name)::GetProcAddress(::GetModuleHandleA("ntdll.dll"), #name)
static GET_FUNC_ADDR(NtWow64ReadVirtualMemory64);
static GET_FUNC_ADDR(NtWow64QueryInformationProcess64);

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

    NTSTATUS status = NtWow64QueryInformationProcess64(handle, sys::ProcessBasicInformation, &pbi, sizeof(pbi), NULL);

    IS_TRUE(NT_SUCCESS(status), "NtQueryInformationProcess failed");
}

uint32_t readPEBData(HANDLE handle, sys::PEB64 *peb) {
    sys::PROCESS_BASIC_INFORMATION64 pbi = {0};
    readPBI(handle, pbi);

    return readMemory64(handle, pbi.PebBaseAddress, sizeof(sys::PEB64), peb);
}

bool getModulesViaPEB(HANDLE handle, const std::function<bool(uint64_t, uint64_t, const wchar_t*)> &cb) {
    sys::PEB64 peb;
    readPEBData(handle, &peb);

    // ------------------------------------------------------------------------
    // Read memory from pointer to loader data structures.
    // ------------------------------------------------------------------------
    std::vector<uint8_t> read_peb_ldr_data(sizeof(sys::PEB_LDR_DATA64));
    readMemory64(handle, (uint64_t)peb.LoaderData, sizeof(sys::PEB_LDR_DATA64), read_peb_ldr_data.data());
    sys::PEB_LDR_DATA64 *peb_ldr_data = (sys::PEB_LDR_DATA64 *)read_peb_ldr_data.data();
    sys::PEB_LDR_DATA64 *loader_data = (sys::PEB_LDR_DATA64 *)peb.LoaderData;

    ULONGLONG address = peb_ldr_data->InLoadOrderModuleList.Flink;

    uint32_t counter = 1;

    // ------------------------------------------------------------------------
    // Traversing loader data structures.
    // ------------------------------------------------------------------------
    for (;;) {
        std::vector<uint8_t> read_ldr_table_entry(sizeof(sys::LDR_DATA_TABLE_ENTRY64));
        readMemory64(handle, address, sizeof(sys::LDR_DATA_TABLE_ENTRY64), read_ldr_table_entry.data());

        sys::LDR_DATA_TABLE_ENTRY64 *ldr_table_entry = (sys::LDR_DATA_TABLE_ENTRY64 *)read_ldr_table_entry.data();
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
/*

int main()
{
    try
    {
        HANDLE handle = get_handle(16944);
        close_on_exit auto_close_handle(handle);

        check_if_process_is_x64(handle);
        get_modules_load_order_via_peb(handle);
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "\n----------------------------------------------------\n";
        std::cerr << "Exception occurred: " << e.what();
        std::cerr << "\n----------------------------------------------------\n";
    }

    return 0;
}
*/
