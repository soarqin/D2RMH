/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "d2rprocess.h"

#include "wow64_process.h"
#include "cfg.h"

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

D2RProcess::D2RProcess(uint32_t searchInterval): searchInterval_(searchInterval) {
    searchForProcess();
    nextSearchTime_ = timeGetTime() + searchInterval_;
}

D2RProcess::~D2RProcess() {
    if (handle_) { CloseHandle(handle_); }
}

void D2RProcess::updateData() {
    if (handle_) {
        DWORD ret = WaitForSingleObject(handle_, 0);
        if (ret != WAIT_TIMEOUT) {
            resetData();
            auto now = timeGetTime();
            bool searchProcess = int(now - nextSearchTime_) >= 0;
            if (searchProcess) {
                searchForProcess();
                nextSearchTime_ = now + searchInterval_;
            }
        }
    } else {
        time_t now = timeGetTime();
        bool searchProcess = int(now - nextSearchTime_) >= 0;
        if (searchProcess) {
            searchForProcess();
            nextSearchTime_ = now + searchInterval_;
        }
    }
    available_ = false;
    if (!handle_) { return; }

    if (playerUnitOffset_ == 0) {
        uint64_t playerPtr = baseAddr_ + 0x20546E0;
        for (uint64_t i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            uint64_t val;
            if (READ(playerPtr, paddr) && paddr && READ(paddr + 0xB8, val) && val == 0x100ULL) {
                playerUnitOffset_ = playerPtr;
                mapEnablePtr_ = baseAddr_ + 0x20643A2;
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
    if (READ(addr + 0x10, plrAddr) && plrAddr) {
        READ(plrAddr, name_);
    }

    uint64_t actAddr = 0;
    if (READ(addr + 0x20, actAddr) && actAddr) {
        READ(actAddr + 0x20, act_);

        uint64_t actUnk1Addr;
        READ(actAddr + 0x70, actUnk1Addr);
        READ(actUnk1Addr + 0x830, difficulty_);
    }

    uint64_t pathAddr;
    if (READ(addr + 0x38, pathAddr) && pathAddr) {
        uint64_t room1Addr;
        if (READ(pathAddr + 0x20, room1Addr) && room1Addr) {
            uint64_t room2Addr;
            if (READ(room1Addr + 0x18, room2Addr) && room2Addr) {
                uint64_t levelAddr;
                if (READ(room2Addr + 0x90, levelAddr) && levelAddr) {
                    READ(levelAddr + 0x1F8, levelId_);
                    if (actAddr) {
                        READ(actAddr + 0x14, seed_);
                    }
                    READ(pathAddr + 0x02, posX_);
                    READ(pathAddr + 0x06, posY_);
                }
            }
        }
    }
    available_ = true;
}

static std::function<void(int, int, int, int)> sizeCallback;

void onSizeChange(HWND hwnd) {
    if (!sizeCallback) { return; }
    RECT rc;
    if (GetClientRect(hwnd, &rc)) {
        POINT pt = {rc.left, rc.top};
        ClientToScreen(hwnd, &pt);
        rc.left = pt.x;
        rc.top = pt.y;
        pt = {rc.right, rc.bottom};
        ClientToScreen(hwnd, &pt);
        rc.right = pt.x;
        rc.bottom = pt.y;
    } else {
        HMONITOR hm = MonitorFromPoint(POINT{1, 1}, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hm, &mi);
        rc = mi.rcWork;
    }
    sizeCallback(rc.left, rc.top, rc.right, rc.bottom);
}

void D2RProcess::setWindowPosCallback(const std::function<void(int, int, int, int)> &cb) {
    sizeCallback = cb;
    onSizeChange((HWND)hwnd_);
}

VOID CALLBACK hookCb(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild,
                     DWORD idEventThread, DWORD dwmsEventTime) {
    onSizeChange(hwnd);
}

void D2RProcess::searchForProcess() {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    DWORD processId = 0;

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
                    processId = entry.th32ProcessID;
                } else {
                    resetData();
                }
                break;
            }
        }
    }
    CloseHandle(snapshot);
    if (!handle_) { return; }
    hook_ = SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND, EVENT_SYSTEM_MOVESIZEEND, nullptr, hookCb, processId, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
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
