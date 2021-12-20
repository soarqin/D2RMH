/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "processmanager.h"

#include "cfg.h"
#include "d2rdefs.h"
#include "util/ntprocess.h"
#include "util/util.h"

#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <cstdio>

namespace d2r {

#include "stringdefs.inl"
uint8_t statsMapping[size_t(StatId::TotalCount)] = {};

static inline void loadEncText(wchar_t *output, const std::wstring &input) {
    if (input.empty()) {
        output[0] = 0;
        return;
    }
    const wchar_t *inp = input.c_str();
    std::wstring out;
    if (inp[0] != '{') {
        out.push_back(char(15));
    }
    while (*inp) {
        if (*inp == '{') {
            wchar_t *endptr;
            out.push_back(char(std::min(15u, uint32_t(std::wcstol(inp + 1, &endptr, 0)))));
            inp = endptr;
            while (*inp && *inp != '}') ++inp;
            if (*inp) {
                ++inp;
            }
        } else {
            out.push_back(*inp++);
        }
    }
    if (out.length() > 3) { out.resize(3); }
    for (auto c: out) {
        *output++ = c;
    }
    *output = 0;
}

enum {
    InventoryPanelOffset = 0x01,
    CharacterPanelOffset = 0x02,
    SkillFloatSelOffset = 0x03,
    SkillTreePanelOffset = 0x04,
    ChatMenuOffset = 0x08,
    SystemMenuOffset = 0x09,
    InGameMapOffset = 0x0A,
    QuestPanelOffset = 0x0E,
    WaypointPanelOffset = 0x13,
    PartyPanelOffset = 0x15,
    MercenaryOffset = 0x1E,
};

#define READ(a, v) readMemory64(currProcess->handle, (a), sizeof(v), &(v))
#define READN(a, v, n) readMemory64(currProcess->handle, (a), (n), (v))

ProcessManager::ProcessManager(uint32_t searchInterval) :
    searchInterval_(std::chrono::milliseconds(searchInterval)) {
    std::pair<StatId, size_t> statsMappingInit[] = {
        {StatId::Damageresist, 0},
        {StatId::Magicresist, 1},
        {StatId::Fireresist, 2},
        {StatId::Lightresist, 3},
        {StatId::Coldresist, 4},
        {StatId::Poisonresist, 5},
    };
    for (auto[k, v]: statsMappingInit) {
        statsMapping[size_t(k)] = v;
    }

    loadFromCfg();
}

void ProcessManager::killProcess() {
    TerminateProcess(HANDLE(currProcess_->handle), 0);
}

inline bool matchMem(size_t sz, const uint8_t *mem, const uint8_t *search, const uint8_t *mask) {
    for (size_t i = 0; i < sz; ++i) {
        uint8_t m = mask[i];
        if ((mem[i] & m) != (search[i] & m)) { return false; }
    }
    return true;
}

inline size_t searchMem(const uint8_t *mem, size_t sz, const uint8_t *search, const uint8_t *mask, size_t searchSz) {
    if (sz < searchSz) { return size_t(-1); }
    size_t e = sz - searchSz + 1;
    for (size_t i = 0; i < e; ++i) {
        if (matchMem(searchSz, mem + i, search, mask)) {
            return i;
        }
    }
    return size_t(-1);
}

static std::function<void(int, int, int, int)> sizeCallback;

void onSizeChange(HWND hwnd) {
    if (!sizeCallback) { return; }
    RECT rc;
    if (GetClientRect(hwnd, &rc)) {
        /* check WS_CAPTION for windowed mode */
        if (GetWindowLong(hwnd, GWL_STYLE) & WS_CAPTION) {
            POINT pt = {rc.left, rc.top};
            if (ClientToScreen(hwnd, &pt)) {
                rc.left = pt.x;
                rc.top = pt.y;
            }
            pt = {rc.right, rc.bottom};
            if (ClientToScreen(hwnd, &pt)) {
                rc.right = pt.x;
                rc.bottom = pt.y;
            }
        }
    } else {
        HMONITOR hm = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hm, &mi);
        rc = mi.rcWork;
    }
    sizeCallback(rc.left, rc.top, rc.right, rc.bottom);
}

void ProcessManager::updateData() {
    auto foregroundWnd = GetForegroundWindow();
    if (!currProcess_ || foregroundWnd != currProcess_->hwnd) {
        for (auto ite = processes_.begin(); ite != processes_.end();) {
            DWORD ret = WaitForSingleObject(ite->second.handle, 0);
            if (ret == WAIT_TIMEOUT) {
                ++ite;
            } else {
                if (processCloseCallback_) {
                    processCloseCallback_(ite->first);
                }
                ite = processes_.erase(ite);
            }
        }
        currProcess_ = nullptr;
        auto now = util::getCurrTime();
        if (now >= nextSearchTime_) {
            searchForProcess(foregroundWnd);
            nextSearchTime_ = now + searchInterval_;
            if (currProcess_ && currProcess_->hwnd) {
                onSizeChange(HWND(currProcess_->hwnd));
            }
        }
    }
    if (!currProcess_) {
        return;
    }
    currProcess_->updateData();
}

void ProcessManager::setWindowPosCallback(const std::function<void(int, int, int, int)> &cb) {
    sizeCallback = cb;
    if (currProcess_) {
        onSizeChange((HWND)currProcess_->hwnd);
    }
}

Skill *ProcessManager::getSkill(uint16_t id) {
    if (!currProcess_) { return nullptr; }
    return currProcess_->getSkill(id);
}

VOID CALLBACK hookCb(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild,
                     DWORD idEventThread, DWORD dwmsEventTime) {
    onSizeChange(hwnd);
}

void ProcessManager::searchForProcess(void *hwnd) {
    if (!hwnd) {
        currProcess_ = nullptr;
        return;
    }
    auto ite = processes_.find(hwnd);
    if (ite != processes_.end()) {
        currProcess_ = &ite->second;
        return;
    }
    DWORD processId = 0;
    if (!GetWindowThreadProcessId((HWND)hwnd, &processId)) { return; }
    HANDLE handle =
        OpenProcess(PROCESS_VM_READ | PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, processId);
    if (!handle) { return; }
    wchar_t fullpath[MAX_PATH];
    if (!GetProcessImageFileNameW(handle, fullpath, MAX_PATH)) {
        CloseHandle(handle);
        return;
    }
    if (StrCmpIW(PathFindFileNameW(fullpath), L"D2R.exe") != 0) {
        CloseHandle(handle);
        return;
    }
    Sleep(1000);
    uint64_t baseAddr, baseSize = 0;
    util::getModules(handle, [&baseAddr, &baseSize](uint64_t addr, uint64_t size, const wchar_t *name) {
        if (StrCmpIW(name, L"D2R.exe") != 0) {
            return true;
        }
        baseAddr = addr;
        baseSize = size;
        return false;
    });
    if (!baseSize) {
        CloseHandle(handle);
        return;
    }
    auto &procInfo = processes_[(void *)hwnd];
    procInfo.handle = handle;
    procInfo.hwnd = hwnd;
    procInfo.baseAddr = baseAddr;
    procInfo.baseSize = baseSize;

    currProcess_ = &procInfo;
    procInfo.hook = SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND,
                                    EVENT_SYSTEM_MOVESIZEEND,
                                    nullptr,
                                    hookCb,
                                    processId,
                                    0,
                                    WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    onSizeChange(HWND(procInfo.hwnd));
    currProcess_->updateOffset();
}

void ProcessManager::loadFromCfg() {
    loadEncText(enchantStrings[5], cfg->encTxtExtraStrong);
    loadEncText(enchantStrings[6], cfg->encTxtExtraFast);
    loadEncText(enchantStrings[7], cfg->encTxtCursed);
    loadEncText(enchantStrings[8], cfg->encTxtMagicResistant);
    loadEncText(enchantStrings[9], cfg->encTxtFireEnchanted);
    loadEncText(enchantStrings[17], cfg->encTxtLigntningEnchanted);
    loadEncText(enchantStrings[18], cfg->encTxtColdEnchanted);
    loadEncText(enchantStrings[25], cfg->encTxtManaBurn);
    loadEncText(enchantStrings[26], cfg->encTxtTeleportation);
    loadEncText(enchantStrings[27], cfg->encTxtSpectralHit);
    loadEncText(enchantStrings[28], cfg->encTxtStoneSkin);
    loadEncText(enchantStrings[29], cfg->encTxtMultipleShots);
    loadEncText(enchantStrings[37], cfg->encTxtFanatic);
    loadEncText(enchantStrings[39], cfg->encTxtBerserker);

    loadEncText(auraStrings[33], cfg->MightAura);
    loadEncText(auraStrings[35], cfg->HolyFireAura);
    loadEncText(auraStrings[40], cfg->BlessedAimAura);
    loadEncText(auraStrings[43], cfg->HolyFreezeAura);
    loadEncText(auraStrings[46], cfg->HolyShockAura);
    loadEncText(auraStrings[28], cfg->ConvictionAura);
    loadEncText(auraStrings[49], cfg->FanaticismAura);

    loadEncText(immunityStrings[0], cfg->encTxtPhysicalImmunity);
    loadEncText(immunityStrings[1], cfg->encTxtMagicImmunity);
    loadEncText(immunityStrings[2], cfg->encTxtFireImmunity);
    loadEncText(immunityStrings[3], cfg->encTxtLightningImmunity);
    loadEncText(immunityStrings[4], cfg->encTxtColdImmunity);
    loadEncText(immunityStrings[5], cfg->encTxtPoisonImmunity);
}

}
