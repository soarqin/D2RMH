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
#include "data.h"
#include "d2rdefs.h"
#include "util.h"

#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <cstdio>

static wchar_t enchantStrings[256][4] = {
    /*  0 */ L"",
    /*  1 */ L"",
    /*  2 */ L"",
    /*  3 */ L"",
    /*  4 */ L"",
    /*  5 */ L"S",
    /*  6 */ L"\x0F" L"F",
    /*  7 */ L"\x02" L"C",
    /*  8 */ L"M",
    /*  9 */ L"\x01" L"FE",
    /* 10 */ L"",
    /* 11 */ L"",
    /* 12 */ L"",
    /* 13 */ L"",
    /* 14 */ L"",
    /* 15 */ L"",
    /* 16 */ L"",
    /* 17 */ L"\x09" L"LE",
    /* 18 */ L"\x03" L"CE",
    /* 19 */ L"",
    /* 20 */ L"",
    /* 21 */ L"",
    /* 22 */ L"",
    /* 23 */ L"",
    /* 24 */ L"",
    /* 25 */ L"\x03" L"MB",
    /* 26 */ L"T",
    /* 27 */ L"H",
    /* 28 */ L"\x04" L"SS",
    /* 29 */ L"\x0C" L"MS",
    /* 30 */ L"A",
    /* 31 */ L"",
    /* 32 */ L"",
    /* 33 */ L"",
    /* 34 */ L"",
    /* 35 */ L"",
    /* 36 */ L"",
    /* 37 */ L"\x0B" L"F",
    /* 38 */ L"",
    /* 39 */ L"\x04" L"B",
    /* 40 */ L"",
    /* 41 */ L"",
    /* 42 */ L"",
    /* 43 */ L"",
};

static wchar_t auraStrings[187][4] = {
    /*   0 */ L"",
    /*   1 */ L"",
    /*   2 */ L"",
    /*   3 */ L"",
    /*   4 */ L"",
    /*   5 */ L"",
    /*   6 */ L"",
    /*   7 */ L"",
    /*   8 */ L"",
    /*   9 */ L"",
    /*  10 */ L"",
    /*  11 */ L"",
    /*  12 */ L"",
    /*  13 */ L"",
    /*  14 */ L"",
    /*  15 */ L"",
    /*  16 */ L"",
    /*  17 */ L"",
    /*  18 */ L"",
    /*  19 */ L"",
    /*  20 */ L"",
    /*  21 */ L"",
    /*  22 */ L"",
    /*  23 */ L"",
    /*  24 */ L"",
    /*  25 */ L"",
    /*  26 */ L"",
    /*  27 */ L"",
    /*  28 */ L"\x0B" L"A",
    /*  29 */ L"",
    /*  30 */ L"",
    /*  31 */ L"",
    /*  32 */ L"",
    /*  33 */ L"\x04" L"A",
    /*  34 */ L"",
    /*  35 */ L"\x01" L"A",
    /*  36 */ L"",
    /*  37 */ L"",
    /*  38 */ L"",
    /*  39 */ L"",
    /*  40 */ L"A",
    /*  41 */ L"",
    /*  42 */ L"",
    /*  43 */ L"\x03" L"A",
    /*  44 */ L"",
    /*  45 */ L"",
    /*  46 */ L"\x09" L"A",
    /*  47 */ L"",
    /*  48 */ L"",
    /*  49 */ L"\x05" L"A",
    /*  50 */ L"",
    /*  51 */ L"",
    /*  52 */ L"",
    /*  53 */ L"",
    /*  54 */ L"",
    /*  55 */ L"",
    /*  56 */ L"",
    /*  57 */ L"",
    /*  58 */ L"",
    /*  59 */ L"",
    /*  60 */ L"",
    /*  61 */ L"",
    /*  62 */ L"",
    /*  63 */ L"",
    /*  64 */ L"",
    /*  65 */ L"",
    /*  66 */ L"",
    /*  67 */ L"",
    /*  68 */ L"",
    /*  69 */ L"",
    /*  70 */ L"",
    /*  71 */ L"",
    /*  72 */ L"",
    /*  73 */ L"",
    /*  74 */ L"",
    /*  75 */ L"",
    /*  76 */ L"",
    /*  77 */ L"",
    /*  78 */ L"",
    /*  79 */ L"",
    /*  80 */ L"",
    /*  81 */ L"",
    /*  82 */ L"",
    /*  83 */ L"",
    /*  84 */ L"",
    /*  85 */ L"",
    /*  86 */ L"",
    /*  87 */ L"",
    /*  88 */ L"",
    /*  89 */ L"",
    /*  90 */ L"",
    /*  91 */ L"",
    /*  92 */ L"",
    /*  93 */ L"",
    /*  94 */ L"",
    /*  95 */ L"",
    /*  96 */ L"",
    /*  97 */ L"",
    /*  98 */ L"",
    /*  99 */ L"",
    /* 100 */ L"",
    /* 101 */ L"",
    /* 102 */ L"",
    /* 103 */ L"",
    /* 104 */ L"",
    /* 105 */ L"",
    /* 106 */ L"",
    /* 107 */ L"",
    /* 108 */ L"",
    /* 109 */ L"",
    /* 110 */ L"",
    /* 111 */ L"",
    /* 112 */ L"",
    /* 113 */ L"",
    /* 114 */ L"",
    /* 115 */ L"",
    /* 116 */ L"",
    /* 117 */ L"",
    /* 118 */ L"",
    /* 119 */ L"",
    /* 120 */ L"",
    /* 121 */ L"",
    /* 122 */ L"",
    /* 123 */ L"",
    /* 124 */ L"",
    /* 125 */ L"",
    /* 126 */ L"",
    /* 127 */ L"",
    /* 128 */ L"",
    /* 129 */ L"",
    /* 130 */ L"",
    /* 131 */ L"",
    /* 132 */ L"",
    /* 133 */ L"",
    /* 134 */ L"",
    /* 135 */ L"",
    /* 136 */ L"",
    /* 137 */ L"",
    /* 138 */ L"",
    /* 139 */ L"",
    /* 140 */ L"",
    /* 141 */ L"",
    /* 142 */ L"",
    /* 143 */ L"",
    /* 144 */ L"",
    /* 145 */ L"",
    /* 146 */ L"",
    /* 147 */ L"",
    /* 148 */ L"",
    /* 149 */ L"",
    /* 150 */ L"",
    /* 151 */ L"",
    /* 152 */ L"",
    /* 153 */ L"",
    /* 154 */ L"",
    /* 155 */ L"",
    /* 156 */ L"",
    /* 157 */ L"",
    /* 158 */ L"",
    /* 159 */ L"",
    /* 160 */ L"",
    /* 161 */ L"",
    /* 162 */ L"",
    /* 163 */ L"",
    /* 164 */ L"",
    /* 165 */ L"",
    /* 166 */ L"",
    /* 167 */ L"",
    /* 168 */ L"",
    /* 169 */ L"",
    /* 170 */ L"",
    /* 171 */ L"",
    /* 172 */ L"",
    /* 173 */ L"",
    /* 174 */ L"",
    /* 175 */ L"",
    /* 176 */ L"",
    /* 177 */ L"",
    /* 178 */ L"",
    /* 179 */ L"",
    /* 180 */ L"",
    /* 181 */ L"",
    /* 182 */ L"",
    /* 183 */ L"",
    /* 184 */ L"",
    /* 185 */ L"",
    /* 186 */ L"",
};

static wchar_t immunityStrings[6][4] = {
    L"\x04" L"i",
    L"\x08" L"i",
    L"\x01" L"i",
    L"\x09" L"i",
    L"\x03" L"i",
    L"\x02" L"i",
};

void loadEncText(wchar_t *output, const std::wstring &input) {
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

static uint8_t statsMapping[size_t(StatId::TotalCount)] = {};

enum {
    /* These 2 offsets are no more used, use memory search now
    HashTableBase = 0x20AF660,
    UIBaseAddr = 0x20BF310,
    */
    InventoryPanelOffset = 0x09,
    CharacterPanelOffset = 0x0A,
    SkillTreePanelOffset = 0x0C,
    ChatMenuOffset = 0x10,
    SystemMenuOffset = 0x11,
    InGameMapOffset = 0x12,
    QuestPanelOffset = 0x16,
    WaypointPanelOffset = 0x1B,
    PartyPanelOffset = 0x1D,
    MercenaryOffset = 0x26,
};

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

#define READ(a, v) readMemory64(currProcess->handle, (a), sizeof(v), &(v))

D2RProcess::D2RProcess(uint32_t searchInterval):
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

inline bool matchMem(size_t sz, const uint8_t* mem, const uint8_t* search, const uint8_t* mask) {
    for (size_t i = 0; i < sz; ++i) {
        uint8_t m = mask[i];
        if ((mem[i] & m) != (search[i] & m)) { return false; }
    }
    return true;
}

inline size_t searchMem(const uint8_t *mem, size_t sz, const uint8_t* search, const uint8_t* mask, size_t searchSz) {
    if (sz < searchSz) { return size_t(-1); }
    size_t e = sz - searchSz + 1;
    for (size_t i = 0; i < e; ++i) {
        if (matchMem(searchSz, mem + i, search, mask)) {
            return i;
        }
    }
    return size_t(-1);
}

void D2RProcess::updateData() {
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
        auto now = getCurrTime();
        if (now >= nextSearchTime_) {
            searchForProcess(foregroundWnd);
            nextSearchTime_ = now + searchInterval_;
        }
    }
    if (!currProcess_) {
        return;
    }

    auto *currProcess = currProcess_;
    uint8_t lastDifficulty;
    uint32_t lastSeed, lastAct, lastLevelId;
    auto *currPlayer = currProcess->currPlayer;
    if (currPlayer) {
        lastDifficulty = currPlayer->difficulty;
        lastSeed = currPlayer->seed;
        lastAct = currPlayer->act;
        lastLevelId = currPlayer->levelId;
    } else {
        lastDifficulty = uint8_t(-1);
        lastSeed = 0;
        lastAct = uint32_t(-1);
        lastLevelId = uint32_t(-1);
    }

    currProcess->mapPlayers.clear();
    if (cfg->showMonsters) {
        currProcess->mapMonsters.clear();
    }
    if (cfg->showItems) {
        currProcess->mapItems.clear();
    }

    uint8_t expansion = 0;
    READ(currProcess->isExpansionAddr, expansion);

    currPlayer = nullptr;
    readUnitHashTable(currProcess->hashTableBaseAddr, [this, expansion, &currPlayer, lastDifficulty, lastSeed, lastAct, lastLevelId](const UnitAny &unit) {
        if (!unit.unitId || !unit.actPtr || !unit.inventoryPtr) { return; }
        auto *currProcess = currProcess_;
        uint64_t token;
        if (!READ(unit.inventoryPtr + (expansion ? 0x70 : 0x30), token) || token == (expansion ? 0 : 1)) {
            /* --START-- remove this if using readRoomUnits() */
            DrlgAct act;
            if (!READ(unit.actPtr, act)) { return; }
            auto &player = currProcess->mapPlayers[unit.unitId];
            player.name[0] = 0;
            READ(unit.unionPtr, player.name);
            player.levelChanged = false;
            player.act = act.actId;
            player.seed = act.seed;
            READ(act.miscPtr + 0x830, player.difficulty);
            DynamicPath path;
            if (!READ(unit.pathPtr, path)) { return; }
            player.posX = path.posX;
            player.posY = path.posY;
            DrlgRoom1 room1;
            if (!READ(path.room1Ptr, room1)) { return; }
            DrlgRoom2 room2;
            if (!READ(room1.room2Ptr, room2)) { return; }
            if (!READ(room2.levelPtr + 0x1F8, player.levelId)) { return; }
            /* --END-- remove this if using readRoomUnits() */
            return;
        }
        DrlgAct act;
        if (!READ(unit.actPtr, act)) { return; }
        auto &player = currProcess->mapPlayers[unit.unitId];
        player.name[0] = 0;
        READ(unit.unionPtr, player.name);
        currProcess->focusedPlayer = unit.unitId;
        currPlayer = &player;
        READ(act.miscPtr + 0x830, player.difficulty);
        player.levelChanged = false;
        player.seed = act.seed;
        if (lastDifficulty != player.difficulty || lastSeed != act.seed) {
            player.levelChanged = true;
            currProcess->knownItems.clear();
        }
        player.act = act.actId;
        if (lastAct != act.actId) {
            player.levelChanged = true;
        }
        if (player.levelChanged) {
            /* get real TalTomb level id */
            READ(act.miscPtr + 0x120, currProcess->realTombLevelId);
            READ(act.miscPtr + 0x874, currProcess->superUniqueTombLevelId);
        }
        DynamicPath path;
        if (!READ(unit.pathPtr, path)) { return; }
        player.posX = path.posX;
        player.posY = path.posY;
        DrlgRoom1 room1;
        if (!READ(path.room1Ptr, room1)) { return; }
        DrlgRoom2 room2;
        if (!READ(room1.room2Ptr, room2)) { return; }
        uint32_t levelId;
        if (!READ(room2.levelPtr + 0x1F8, levelId)) { return; }

        player.levelId = levelId;
        if (levelId != lastLevelId) {
            player.levelChanged = true;
        }
        if (player.levelChanged) {
            if (cfg->showObjects) {
                currProcess->mapObjects.clear();
            }
        }
/*  this is another way of reading all units from memory, we may need this if we need to see units on neighbour levels
        std::unordered_set<uint64_t> rset(256);
        rset.insert(path.room1Ptr);
        readRoomUnits(room1, rset);
*/
    });
    if (currProcess->mapPlayers.empty()) {
        currProcess->focusedPlayer = 0;
        return;
    }
    if (!currProcess->focusedPlayer) {
        auto ite = currProcess->mapPlayers.begin();
        currProcess->focusedPlayer = ite->first;
        currPlayer = &ite->second;
    } else if (!currPlayer) {
        auto ite = currProcess->mapPlayers.find(currProcess->focusedPlayer);
        if (ite == currProcess->mapPlayers.end()) {
            ite = currProcess->mapPlayers.begin();
            currProcess->focusedPlayer = ite->first;
        }
        currPlayer = &ite->second;
    }
    uint8_t mem[0x28];
    READ(currProcess->uiBaseAddr, mem);
    currProcess->mapEnabled = mem[InGameMapOffset];
    uint32_t panelEnabled = 0;
    if (mem[InventoryPanelOffset]) { panelEnabled |= 0x01; }
    if (mem[CharacterPanelOffset]) { panelEnabled |= 0x02; }
    if (mem[SkillTreePanelOffset]) { panelEnabled |= 0x04; }
    if (mem[SystemMenuOffset]) { panelEnabled |= 0x08; }
    if (mem[QuestPanelOffset]) { panelEnabled |= 0x10; }
    if (mem[PartyPanelOffset]) { panelEnabled |= 0x20; }
    if (mem[MercenaryOffset]) { panelEnabled |= 0x40; }
    if (mem[WaypointPanelOffset]) { panelEnabled |= 0x80; }
    currProcess->panelEnabled = panelEnabled;
    currProcess->currPlayer = currPlayer;

    if (cfg->showMonsters) {
        readUnitHashTable(currProcess->hashTableBaseAddr + 8 * 0x80, [this](const auto &unit) {
            readUnit(unit);
        });
    }
    if (cfg->showObjects) {
        readUnitHashTable(currProcess->hashTableBaseAddr + 8 * 0x100, [this](const auto &unit) {
            readUnit(unit);
        });
    }
    if (cfg->showItems) {
        readUnitHashTable(currProcess->hashTableBaseAddr + 8 * 0x200, [this](const auto &unit) {
            readUnit(unit);
        });
    }
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
    if (currProcess_) {
        onSizeChange((HWND)currProcess_->hwnd);
    }
}

VOID CALLBACK hookCb(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild,
                     DWORD idEventThread, DWORD dwmsEventTime) {
    onSizeChange(hwnd);
}

void D2RProcess::searchForProcess(void *hwnd) {
    if (!hwnd) { currProcess_ = nullptr; return; }
    auto ite = processes_.find(hwnd);
    if (ite != processes_.end()) {
        currProcess_ = &ite->second;
        onSizeChange(HWND(currProcess_->hwnd));
        return;
    }
    DWORD processId = 0;
    if (!GetWindowThreadProcessId((HWND)hwnd, &processId)) { return; }
    HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, processId);
    if (!handle) { return; }
    wchar_t fullpath[MAX_PATH];
    if (!GetProcessImageFileNameW(handle, fullpath, MAX_PATH)) { CloseHandle(handle); return; }
    if (StrCmpIW(PathFindFileNameW(fullpath), L"D2R.exe") != 0) { CloseHandle(handle); return; }
    Sleep(1000);
    uint64_t baseAddr, baseSize = 0;
    getModulesViaPEB(handle, [&baseAddr, &baseSize](uint64_t addr, uint64_t size, const wchar_t *name) {
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
    auto &procInfo = processes_[(void*)hwnd];
    procInfo.handle = handle;
    procInfo.hwnd = hwnd;
    procInfo.baseAddr = baseAddr;
    procInfo.baseSize = baseSize;

    /*
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
                    processId = entry.th32ProcessID;
                } else {
                    resetData();
                }
                break;
            }
        }
    }
    CloseHandle(snapshot);
     */
    currProcess_ = &procInfo;
    procInfo.hook = SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND, EVENT_SYSTEM_MOVESIZEEND, nullptr, hookCb, processId, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    onSizeChange(HWND(procInfo.hwnd));
    updateOffset();
    updateData();
}

void D2RProcess::updateOffset() {
    auto *currProcess = currProcess_;
    auto *mem = new(std::nothrow) uint8_t[currProcess->baseSize];
    if (mem && readMemory64(currProcess->handle, currProcess->baseAddr, currProcess->baseSize, mem)) {
        const uint8_t search0[] = {0x48, 0x8D, 0, 0, 0, 0, 0, 0x8B, 0xD1};
        const uint8_t mask0[] = {0xFF, 0xFF, 0, 0, 0, 0, 0, 0xFF, 0xFF};
        auto off = searchMem(mem, currProcess->baseSize, search0, mask0, sizeof(search0));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(currProcess->baseAddr + off + 3, rel)) {
                currProcess->hashTableBaseAddr = currProcess->baseAddr + off + 7 + rel;
            }
        }

        const uint8_t search1[] = {0x40, 0x84, 0xED, 0x0F, 0x94, 0x05};
        const uint8_t mask1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        off = searchMem(mem, currProcess->baseSize, search1, mask1, sizeof(search1));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(currProcess->baseAddr + off + 6, rel)) {
                currProcess->uiBaseAddr = currProcess->baseAddr + off + 10 + rel - 0x12;
            }
        }
        const uint8_t search2[] = {0xC7, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x83, 0x78, 0x5C, 0x00, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x33, 0xD2, 0x41};
        const uint8_t mask2[] = {0xFF, 0xFF, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF};
        off = searchMem(mem, currProcess->baseSize, search2, mask2, sizeof(search2));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(currProcess->baseAddr + off - 4, rel)) {
                currProcess->isExpansionAddr = currProcess->baseAddr + off + rel;
            }
        }
    }
    delete[] mem;
}

void D2RProcess::readUnitHashTable(uint64_t addr, const std::function<void(const UnitAny&)> &callback) {
    auto *currProcess = currProcess_;
    uint64_t addrList[0x80];
    if (!READ(addr, addrList)) { return; }
    for (auto paddr: addrList) {
        while (paddr) {
            UnitAny unit;
            if (READ(paddr, unit)) {
                callback(unit);
            }
            paddr = unit.nextPtr;
        }
    }
}
void D2RProcess::readStateList(uint64_t addr, uint32_t unitId, const std::function<void(const StatList &)> &callback) {
    auto *currProcess = currProcess_;
    StatList stats;
    if (!READ(addr, stats)) { return; }
    do {
        /* check if this is owner stat or aura */
        if (stats.ownerId == unitId) {
            callback(stats);
        }
        if (!(stats.flag & 0x80000000u)) { break; }
        if (!stats.nextListEx || !READ(stats.nextListEx, stats)) { break; }
    } while (true);
}

void D2RProcess::readRoomUnits(const DrlgRoom1 &room1, std::unordered_set<uint64_t> &roomList) {
    auto *currProcess = currProcess_;
    DrlgRoom2 room2;
    READ(room1.room2Ptr, room2);
    uint64_t roomsNear[64];
    uint64_t addr = room1.unitFirstAddr;
    while (addr) {
        UnitAny unit;
        if (!READ(addr, unit)) { break; }
        readUnit(unit);
        addr = unit.roomNextPtr;
    }
    auto count = std::min(64u, room1.roomsNear);
    if (readMemory64(currProcess_->handle, room1.roomsNearListPtr, count * sizeof(uint64_t), roomsNear)) {
        auto *currProcess = currProcess_;
        for (auto i = 0u; i < count; ++i) {
            auto addr = roomsNear[i];
            if (roomList.find(addr) != roomList.end()) { continue; }
            DrlgRoom1 room;
            if (!READ(addr, room)) { continue; }
            roomList.insert(addr);
            readRoomUnits(room, roomList);
        }
    }
}

void D2RProcess::readUnit(const UnitAny &unit) {
    switch (unit.unitType) {
    case 0:
        readUnitPlayer(unit);
        break;
    case 1:
        if (cfg->showMonsters) { readUnitMonster(unit); }
        break;
    case 2:
        if (cfg->showObjects) { readUnitObject(unit); }
        break;
    case 4:
        if (cfg->showItems) { readUnitItem(unit); }
        break;
    }
}
void D2RProcess::readUnitPlayer(const UnitAny &unit) {
    auto *currProcess = currProcess_;
    if (unit.unitId == currProcess->focusedPlayer) { return; }
    DrlgAct act;
    if (!READ(unit.actPtr, act)) { return; }
    auto &player = currProcess->mapPlayers[unit.unitId];
    player.name[0] = 0;
    READ(unit.unionPtr, player.name);
    player.levelChanged = false;
    player.act = act.actId;
    player.seed = act.seed;
    READ(act.miscPtr + 0x830, player.difficulty);
    DynamicPath path;
    if (!READ(unit.pathPtr, path)) { return; }
    player.posX = path.posX;
    player.posY = path.posY;
    DrlgRoom1 room1;
    if (!READ(path.room1Ptr, room1)) { return; }
    DrlgRoom2 room2;
    if (!READ(room1.room2Ptr, room2)) { return; }
    if (!READ(room2.levelPtr + 0x1F8, player.levelId)) { return; }
}
void D2RProcess::readUnitMonster(const UnitAny &unit) {
    if (unit.mode == 0 || unit.mode == 12 || unit.txtFileNo >= gamedata->monsters.size()) { return; }
    auto *currProcess = currProcess_;
    MonsterData monData;
    if (!READ(unit.unionPtr, monData)) { return; }
    auto isUnique = (monData.flag & 0x0E) != 0;
    auto &txtData = gamedata->monsters[unit.txtFileNo];
    auto isNpc = std::get<1>(txtData);
    auto sm = cfg->showMonsters;
    if (!isNpc && !(sm == 2 || (isUnique && sm == 1))) { return; }
    DynamicPath path;
    if (!READ(unit.pathPtr, path)) { return; }
    auto &mon = currProcess->mapMonsters.emplace_back();
    mon.x = path.posX;
    mon.y = path.posY;
    mon.isNpc = isNpc;
    mon.isUnique = isUnique;
    mon.flag = monData.flag;
    if (isNpc) {
        if (cfg->showNpcNames) {
            if (monData.mercNameId == uint16_t(-1) || monData.mercNameId >= gamedata->mercNames.size()) {
                mon.name = std::get<2>(txtData);
            } else {
                mon.name = gamedata->mercNames[monData.mercNameId].second;
            }
        }
        return;
    } else if (auto sn = cfg->showMonsterNames; (sn == 2 || (isUnique && sn == 1))) {
        /* Super unique */
        if ((monData.flag & 2) && monData.uniqueNo < gamedata->superUniques.size()) {
            mon.name = gamedata->superUniques[monData.uniqueNo].second;
        } else {
            mon.name = std::get<2>(txtData);
        }
    }
    int off = 0;
    bool hasAura = false;
    if (auto sme = cfg->showMonsterEnchants; sme == 2 || (isUnique && sme == 1)) {
        uint8_t id;
        for (int n = 0; n < 9 && (id = monData.enchants[n]) != 0; ++n) {
            if (id == 30) {
                hasAura = true;
                continue;
            }
            const auto *str = enchantStrings[id];
            while (*str) {
                mon.enchants[off++] = *str++;
            }
        }
    }
    auto smi = cfg->showMonsterImmunities;
    bool showMI = smi == 2 || (isUnique && smi == 1);
    if (!showMI && !hasAura) {
        mon.enchants[off] = 0;
        return;
    }
    readStateList(unit.statListPtr, unit.unitId, [this, &off, &mon, hasAura, showMI](const StatList &stats) {
        if (stats.stateNo) {
            if (!hasAura) { return; }
            const wchar_t *str = auraStrings[stats.stateNo];
            while (*str) {
                mon.enchants[off++] = *str++;
            }
            return;
        }
        if (!showMI) { return; }
        auto *currProcess = currProcess_;
        static StatEx statEx[64];
        auto cnt = std::min(64u, uint32_t(stats.stat.statCount));
        if (!readMemory64(currProcess->handle, stats.stat.statPtr, sizeof(StatEx) * cnt, statEx)) { return; }
        StatEx *st = statEx;
        for (; cnt; --cnt, ++st) {
            if (st->value < 100) { continue; }
            auto statId = st->stateId;
            if (statId >= uint16_t(StatId::TotalCount)) { continue; }
            auto mapping = statsMapping[statId];
            if (!mapping) { continue; }
            const wchar_t *str = immunityStrings[mapping];
            while (*str) {
                mon.enchants[off++] = *str++;
            }
        }
    });
    mon.enchants[off] = 0;
}

void D2RProcess::readUnitObject(const UnitAny &unit) {
    auto *currProcess = currProcess_;
    if (/* Portals */ unit.txtFileNo != 59 && unit.txtFileNo != 60 && /* destroyed/opened */ unit.mode == 2) {
        StaticPath path;
        if (READ(unit.pathPtr, path)) {
            currProcess->mapObjects.erase(path.posX | (uint32_t(path.posY) << 16));
        }
        return;
    }
    auto ite = gamedata->objects[0].find(unit.txtFileNo);
    if (ite == gamedata->objects[0].end()) { return; }
    auto type = std::get<0>(ite->second);
    switch (type) {
    case TypePortal:
    case TypeWell:
    case TypeShrine: {
        uint8_t flag;
        READ(unit.unionPtr + 8, flag);
        StaticPath path;
        if (!READ(unit.pathPtr, path)) { break; }
        auto &obj = currProcess->mapObjects[path.posX | (uint32_t(path.posY) << 16)];
        if (obj.x) { break; }
        obj.type = std::get<0>(ite->second);
        obj.name = type == TypeShrine && flag < gamedata->shrines.size() ?
                   gamedata->shrines[flag].second : std::get<2>(ite->second);
        obj.x = path.posX;
        obj.y = path.posY;
        obj.flag = flag;
        obj.w = std::get<3>(ite->second) * .5f;
        obj.h = std::get<4>(ite->second) * .5f;
        break;
    }
    default:
        break;
    }
}

void D2RProcess::readUnitItem(const UnitAny &unit) {
    auto *currProcess = currProcess_;
    ItemData item;
    if (!unit.actPtr /* ground items has pAct set */
        || !READ(unit.unionPtr, item)
        || item.ownerInvPtr || item.location != 0
        || (item.itemFlags & 0x01) /* InStore */)
    { return; }
    /* the item is on the ground */
    uint32_t sockets = 0;
    if (item.itemFlags & 0x800u) {
        readStateList(unit.statListPtr, unit.unitId, [this, &sockets](const StatList &stats) {
            if (stats.stateNo) { return; }
            auto *currProcess = currProcess_;
            static StatEx statEx[64];
            auto cnt = std::min(64u, uint32_t(stats.stat.statCount));
            if (!readMemory64(currProcess->handle, stats.stat.statPtr, sizeof(StatEx) * cnt, statEx)) { return; }
            StatEx *st = statEx;
            for (; cnt; --cnt, ++st) {
                if (st->stateId != uint16_t(StatId::ItemNumsockets)) { continue; }
                sockets = st->value;
                break;
            }
        });
    }
    auto n = filterItem(&unit, &item, sockets);
    if (!n) { return; }
    StaticPath path;
    if (!READ(unit.pathPtr, path)) { return; }
    auto &mitem = currProcess->mapItems.emplace_back();
    mitem.x = path.posX;
    mitem.y = path.posY;
    mitem.name = unit.txtFileNo < gamedata->items.size() ? gamedata->items[unit.txtFileNo].second : nullptr;
    mitem.flag = n & 0x0F;
    auto color = (n >> 4) & 0x0F;
    if (color == 0) {
        if (auto quality = uint32_t(item.quality - 1); quality < 8) {
            const uint8_t qualityColors[8] = {15, 15, 5, 3, 2, 9, 4, 8};
            color = qualityColors[quality];
        }
    }
    mitem.color = color;
    auto snd = n >> 8;
    if (snd > 0 && currProcess->knownItems.find(unit.unitId) == currProcess->knownItems.end()) {
        currProcess->knownItems.insert(unit.unitId);
        if (snd < cfg->sounds.size() && !cfg->sounds[snd].first.empty()) {
            if (cfg->sounds[snd].second) {
                PlaySoundW(cfg->sounds[snd].first.c_str(), nullptr, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
            } else {
                PlaySoundW(cfg->sounds[snd].first.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            }
        }
    }
}
void D2RProcess::loadFromCfg() {
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

D2RProcess::ProcessData::ProcessData():
    mapObjects(1024) {
    mapMonsters.reserve(1024);
    mapItems.reserve(1024);
}
D2RProcess::ProcessData::~ProcessData() {
    if (handle) {
        UnhookWinEvent(HWINEVENTHOOK(hook));
        hook = nullptr;
        CloseHandle(handle);
    }
}
void D2RProcess::ProcessData::resetData() {
    if (handle) {
        UnhookWinEvent(HWINEVENTHOOK(hook));
        hook = nullptr;
        CloseHandle(handle);
        handle = nullptr;
    }

    hwnd = nullptr;

    baseAddr = 0;
    baseSize = 0;

    mapEnabled = 0;
    focusedPlayer = 0;
    currPlayer = nullptr;

    mapPlayers.clear();
    mapMonsters.clear();
    mapObjects.clear();
    mapItems.clear();
}
