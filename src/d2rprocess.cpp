/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "d2rprocess.h"

#include "wow64_process.h"

#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <cstdio>

struct handle_data {
    unsigned long processId;
    HWND hwnd;
};

BOOL isMainWindow(HWND handle) {
    return GetWindow(handle, GW_OWNER) == (HWND)nullptr && IsWindowVisible(handle);
}

BOOL CALLBACK enumWindowsCallback(HWND handle, LPARAM lParam) {
    handle_data &data = *(handle_data *)lParam;
    unsigned long processId = 0;
    GetWindowThreadProcessId(handle, &processId);
    if (data.processId != processId || !isMainWindow(handle))
        return TRUE;
    data.hwnd = handle;
    return FALSE;
}

HWND findMainWindow(unsigned long processId) {
    handle_data data = {processId, nullptr };
    EnumWindows(enumWindowsCallback, (LPARAM)&data);
    return data.hwnd;
}

#define READ(a, v) readMemory64(handle_, (a), sizeof(v), &(v))

D2RProcess::D2RProcess() {
    searchForProcess();
}

D2RProcess::~D2RProcess() {
    if (handle_) { CloseHandle(handle_); }
}

void D2RProcess::updateData(bool searchProcess) {
    if (handle_) {
        DWORD ret = WaitForSingleObject(handle_, 0);
        if (ret != WAIT_TIMEOUT) {
            resetData();
            if (searchProcess) { searchForProcess(); }
        }
    } else {
        if (searchProcess) { searchForProcess(); }
    }
    available_ = false;
    if (!handle_) { return; }

    if (playerUnitOffset_ == 0) {
        uint64_t playerPtr = baseAddr_ + 0x2027660;
        for (uint64_t i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            uint64_t val;
            if (READ(playerPtr, paddr) && paddr && READ(paddr + 0xB8, val) && val == 0x100ULL) {
                playerUnitOffset_ = playerPtr;
                mapEnablePtr_ = baseAddr_ + 0x2037322;
                break;
            }
            playerPtr += 8;
        }
    }

    READ(mapEnablePtr_, mapEnabled_);

    uint64_t addr;
    if (!READ(playerUnitOffset_, addr) || !addr) {
        playerUnitOffset_ = 0;
        return;
    }

    uint64_t plrAddr;
    READ(addr + 0x10, plrAddr);
    uint64_t actAddr;
    READ(addr + 0x20, actAddr);

    READ(plrAddr, name_);

    READ(actAddr + 0x20, act_);

    uint64_t actUnk1Addr;
    READ(actAddr + 0x70, actUnk1Addr);
    READ(actUnk1Addr + 0x830, difficulty_);

    uint64_t pathAddr;
    READ(addr + 0x38, pathAddr);
    uint64_t room1Addr;
    READ(pathAddr + 0x20, room1Addr);
    uint64_t room2Addr;
    READ(room1Addr + 0x18, room2Addr);
    uint64_t levelAddr;
    READ(room2Addr + 0x90, levelAddr);
    if (levelAddr != 0) {
        READ(levelAddr + 0x1F8, levelId_);
        READ(actAddr + 0x14, seed_);
        READ(pathAddr + 0x02, posX_);
        READ(pathAddr + 0x06, posY_);
    }
    available_ = true;
}

void D2RProcess::searchForProcess() {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32FirstW(snapshot, &entry) == TRUE) {
        while (Process32NextW(snapshot, &entry) == TRUE) {
            if (StrCmpIW(entry.szExeFile, L"D2R.exe") == 0) {
                handle_ = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, entry.th32ProcessID);
                if (!handle_) { break; }
                Sleep(1000);
                getModulesViaPEB(handle_, [this](uint64_t addr, uint64_t size, const wchar_t *name) {
                    if (StrCmpIW(name, L"D2R.exe") != 0) {
                        return true;
                    }
                    baseAddr_ = addr;
                    baseSize_ = size;
                    return false;
                });
                if (baseSize_) {
                    hwnd_ = findMainWindow(entry.th32ProcessID);
                } else {
                    resetData();
                }
                break;
            }
        }
    }
    CloseHandle(snapshot);
    if (!handle_) { return; }
/*
    auto *data = new uint8_t[baseSize_];
    if (!READN(baseAddr_, baseSize_, data)) {
        resetData();
        delete[] data;
        return;
    }
    const uint8_t seq0[] = {0x57, 0x48, 0x83, 0xec, 0x00, 0x33, 0xff, 0x48, 0x8d, 0x05};
    const uint8_t mask0[] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    auto ptr1 = findPattern(handle_, data, baseSize_, seq0, mask0, 10);
    const uint8_t seq1[] = {0x40, 0x84, 0xed, 0x0f, 0x94, 0x05};
    auto ptr2 = findPattern(handle_, data, baseSize_, seq1, nullptr, 6);

    delete[] data;
*/

    updateData();
}

void D2RProcess::resetData() {
    if (handle_) {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
    available_ = false;

    hwnd_ = nullptr;

    baseAddr_ = 0;
    baseSize_ = 0;

    playerUnitOffset_ = 0;
    mapEnablePtr_ = 0;

    mapEnabled_ = 0;
    name_[0] = 0;
    act_ = 0;
    difficulty_ = 0;
    levelId_ = 0;
    seed_ = 0;
    posX_ = 0;
    posY_ = 0;
}
