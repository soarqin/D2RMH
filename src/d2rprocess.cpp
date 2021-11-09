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
#include <tlhelp32.h>
#include <shlwapi.h>
#include <cstdio>

static wchar_t enchantStrings[256][4] = {
    /*  0 */ L"",
    /*  1 */ L"",
    /*  2 */ L"",
    /*  3 */ L"",
    /*  4 */ L"",
    /*  5 */ L"S",
    /*  6 */ L"\x0FF",
    /*  7 */ L"\x02C",
    /*  8 */ L"M",
    /*  9 */ L"\x01FE",
    /* 10 */ L"",
    /* 11 */ L"",
    /* 12 */ L"",
    /* 13 */ L"",
    /* 14 */ L"",
    /* 15 */ L"",
    /* 16 */ L"",
    /* 17 */ L"\x09LE",
    /* 18 */ L"\x03CE",
    /* 19 */ L"",
    /* 20 */ L"",
    /* 21 */ L"",
    /* 22 */ L"",
    /* 23 */ L"",
    /* 24 */ L"",
    /* 25 */ L"\x03MB",
    /* 26 */ L"T",
    /* 27 */ L"H",
    /* 28 */ L"\x04SS",
    /* 29 */ L"\x0CMS",
    /* 30 */ L"A",
    /* 31 */ L"",
    /* 32 */ L"",
    /* 33 */ L"",
    /* 34 */ L"",
    /* 35 */ L"",
    /* 36 */ L"",
    /* 37 */ L"\x0BF",
    /* 38 */ L"",
    /* 39 */ L"\x04B",
    /* 40 */ L"",
    /* 41 */ L"",
    /* 42 */ L"",
    /* 43 */ L"",
};

static wchar_t auraStrings[6][4] = {
    L"\x04i",
    L"\x08i",
    L"\x01i",
    L"\x09i",
    L"\x03i",
    L"\x02i",
};

void loadEncText(wchar_t *output, const std::string &input) {
    if (input.empty()) {
        output[0] == 0;
        return;
    }
    const char *inp = input.c_str();
    std::string out;
    if (inp[0] != '{') {
        out.push_back(char(15));
    }
    while (*inp) {
        if (*inp == '{') {
            char *endptr;
            out.push_back(char(std::min(15u, uint32_t(std::strtol(inp + 1, &endptr, 0)))));
            inp = endptr;
            while (*inp && *inp != '}') ++inp;
            if (*inp) {
                ++inp;
            }
        } else {
            out.push_back(*inp++);
        }
    }
    auto outw = utf8toucs4(out);
    if (outw.length() > 3) { outw.resize(3); }
    for (auto c: outw) {
        *output++ = c;
    }
    *output = 0;
}

static uint8_t statsMapping[size_t(StatId::TotalCount)] = {};

enum {
    HashTableBase = 0x20546E0,
    MapEnabledAddr = 0x20643A2,
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

#define READ(a, v) readMemory64(handle_, (a), sizeof(v), &(v))

D2RProcess::D2RProcess(uint32_t searchInterval): searchInterval_(searchInterval) {
    searchForProcess();
    nextSearchTime_ = timeGetTime() + searchInterval_;
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
    loadEncText(enchantStrings[30], cfg->encTxtAura);
    loadEncText(auraStrings[0], cfg->encTxtPhysicalImmunity);
    loadEncText(auraStrings[1], cfg->encTxtMagicImmunity);
    loadEncText(auraStrings[2], cfg->encTxtFireImmunity);
    loadEncText(auraStrings[3], cfg->encTxtLightningImmunity);
    loadEncText(auraStrings[4], cfg->encTxtColdImmunity);
    loadEncText(auraStrings[5], cfg->encTxtPoisonImmunity);
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
        auto now = timeGetTime();
        bool searchProcess = int(now - nextSearchTime_) >= 0;
        if (searchProcess) {
            searchForProcess();
            nextSearchTime_ = now + searchInterval_;
        }
    }
    available_ = false;
    if (!handle_) { return; }

    if (playerHashOffset_ == 0) {
        uint64_t playerPtr = baseAddr_ + HashTableBase;
        for (int i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            if (!READ(playerPtr, paddr)) { return; }
            while (paddr) {
                UnitAny unit;
                if (READ(paddr, unit) && unit.actPtr && unit.inventoryPtr && unit.unk5[4] == 0x100) {
                    playerHashOffset_ = playerPtr;
                    playerPtrOffset_ = paddr;
                    break;
                }
                paddr = unit.nextPtr;
            }
            if (playerHashOffset_) {
                break;
            }
            playerPtr += 8;
        }
        if (!playerHashOffset_) {
            return;
        }
    }

    READ(baseAddr_ + MapEnabledAddr, mapEnabled_);
    if (uint64_t addr; !READ(playerHashOffset_, addr) || !addr) {
        playerHashOffset_ = 0;
        return;
    }

    UnitAny unit;
    if (!READ(playerPtrOffset_, unit)) {
        playerHashOffset_ = 0;
        return;
    }
    DrlgAct act;
    if (!READ(unit.actPtr, act)) {
        playerHashOffset_ = 0;
        return;
    }
    name_[0] = 0;
    bool levelChanged = false;
    READ(unit.unionPtr, name_);
    act_ = act.actId;
    seed_ = act.seed;
    READ(act.miscPtr + 0x830, difficulty_);
    DynamicPath path;
    if (READ(unit.pathPtr, path)) {
        posX_ = path.posX;
        posY_ = path.posY;
        DrlgRoom1 room1;
        if (READ(path.room1Ptr, room1)) {
            DrlgRoom2 room2;
            if (READ(room1.room2Ptr, room2)) {
                uint32_t levelId;
                if (READ(room2.levelPtr + 0x1F8, levelId) && levelId_ != levelId) {
                    levelChanged = true;
                    levelId_ = levelId;
                }
            }
        }
    }
    available_ = true;

    if (cfg->showMonsters) {
        mapMonsters_.clear();
        uint64_t baseAddr = baseAddr_ + HashTableBase + 8 * 0x80;
        auto showName = cfg->showMonsterName;
        auto showEnchant = cfg->showMonsterEnchant;
        auto showImmune = cfg->showMonsterImmune;
        for (int i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            if (!READ(baseAddr, paddr)) { break; }
            while (paddr) {
                UnitAny unit;
                do {
                    if (!READ(paddr, unit)) { break; }
                    if (unit.mode == 0 || unit.mode == 12) { break; }
                    MonsterData monData;
                    if (!READ(unit.unionPtr, monData)) { break; }
                    /*
                    for flag:
                    struct {
                        BYTE fOther:1;  //set for some champs, uniques
                        BYTE fUnique:1; //super unique
                        BYTE fChamp:1;
                        BYTE fBoss:1;   //unique monster ,usually boss
                        BYTE fMinion:1;
                        BYTE fPoss:1;   //possessed
                        BYTE fGhost:1;  //ghostly
                        BYTE fMulti:1;  //multishotfiring
                    };
                    */
                    if (!(monData.flag & 0x0E)) { break; }
                    DynamicPath path;
                    if (READ(unit.pathPtr, path)) {
                        auto &mon = mapMonsters_[unit.unitId];
                        mon.x = path.posX;
                        mon.y = path.posY;
                        if (!mon.txtFileNo) {
                            mon.txtFileNo = unit.txtFileNo;
                            mon.flag = monData.flag;
                        }
                        if (showName) {
                            /* Super unique */
                            if (monData.flag & 2) {
                                if (monData.uniqueNo < gamedata->superUniques.size()) {
                                    auto ite2 = gamedata->strings.find(gamedata->superUniques[monData.uniqueNo]);
                                    mon.name = ite2 != gamedata->strings.end() ? &ite2->second : nullptr;
                                }
                            }
                        }
                        int off = 0;
                        if (showEnchant) {
                            for (int n = 0; n < 9 && monData.enchants[n] != 0; ++n) {
                                const auto *str = enchantStrings[monData.enchants[n]];
                                while(*str) {
                                    mon.enchants[off++] = *str++;
                                }
                            }
                        }
                        if (showImmune) {
                            if (StatList stats; READ(unit.statListPtr, stats)) {
                                static StatEx statEx[64];
                                auto cnt = std::min(64u, stats.stat.statCount);
                                if (readMemory64(handle_, stats.stat.statPtr, sizeof(StatEx) * cnt, statEx)) {
                                    StatEx *st = statEx;
                                    for (; cnt; --cnt, ++st) {
                                        if (st->value < 100) { continue; }
                                        if (auto statId = st->stateId; statId < uint16_t(StatId::TotalCount)) {
                                            if (auto mapping = statsMapping[statId]; mapping) {
                                                const wchar_t *str = auraStrings[mapping];
                                                while(*str) {
                                                    mon.enchants[off++] = *str++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            mon.enchants[off] = 0;
                        }
                    }
                } while (false);
                paddr = unit.nextPtr;
            }
            baseAddr += 8;
        }
    }
    if (cfg->showObjects) {
        if (levelChanged) {
            mapObjects_.clear();
        }
        uint64_t baseAddr = baseAddr_ + HashTableBase + 8 * 0x100;
        for (int i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            if (!READ(baseAddr, paddr)) { break; }
            while (paddr) {
                UnitAny unit;
                if (!READ(paddr, unit)) { break; }
                if (unit.mode != 2) {
                    auto ite = gamedata->objects[0].find(unit.txtFileNo);
                    if (ite != gamedata->objects[0].end()) {
                        auto type = ite->second.type;
                        switch (type) {
                        case TypeWell:
                        case TypeShrine: {
                            uint8_t flag;
                            READ(unit.unionPtr + 8, flag);
                            StaticPath path;
                            if (!READ(unit.pathPtr, path)) { break; }
                            auto &obj = mapObjects_[path.posX | (uint32_t(path.posY) << 16)];
                            if (obj.txtFileNo) { break; }
                            obj.txtFileNo = unit.txtFileNo;
                            obj.type = ite->second.type;
                            const std::string &name = type == TypeShrine && flag < gamedata->shrines.size() ?
                                gamedata->shrines[flag] : ite->second.name;
                            auto ite2 = gamedata->strings.find(name);
                            obj.name = ite2 != gamedata->strings.end() ? &ite2->second : nullptr;
                            obj.x = path.posX;
                            obj.y = path.posY;
                            obj.flag = flag;
                            break;
                        }
                        default:
                            break;
                        }
                    }
                } else {
                    StaticPath path;
                    if (READ(unit.pathPtr, path)) {
                        mapObjects_.erase(path.posX | (uint32_t(path.posY) << 16));
                    }
                }
                paddr = unit.nextPtr;
            }
            baseAddr += 8;
        }
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

    playerHashOffset_ = 0;

    mapEnabled_ = 0;
    name_[0] = 0;
    act_ = 0;
    difficulty_ = 0;
    levelId_ = 0;
    seed_ = 0;
    posX_ = 0;
    posY_ = 0;

    mapMonsters_.clear();
    mapObjects_.clear();
}
