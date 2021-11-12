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
        auto now = timeGetTime();
        bool searchProcess = int(now - nextSearchTime_) >= 0;
        if (searchProcess) {
            searchForProcess();
            nextSearchTime_ = now + searchInterval_;
        }
    }
    available_ = false;
    if (!handle_) { return; }

    mapPlayers_.clear();

    uint64_t playerPtr = baseAddr_ + HashTableBase;
    for (int i = 0; i < 0x80; ++i) {
        uint64_t paddr;
        if (!READ(playerPtr, paddr)) { return; }
        while (paddr) {
            UnitAny unit;
            if (READ(paddr, unit) && unit.unitId && unit.actPtr && unit.inventoryPtr) {
                if (DrlgAct act; READ(unit.actPtr, act)) {
                    auto &player = mapPlayers_[unit.unitId];
                    player.name[0] = 0;
                    bool levelChanged = false;
                    READ(unit.unionPtr, player.name);
                    player.levelChanged = player.act != act.actId;
                    player.act = act.actId;
                    player.seed = act.seed;
                    READ(act.miscPtr + 0x830, player.difficulty);
                    if (DynamicPath path; READ(unit.pathPtr, path)) {
                        player.posX = path.posX;
                        player.posY = path.posY;
                        if (DrlgRoom1 room1; READ(path.room1Ptr, room1)) {
                            if (DrlgRoom2 room2; READ(room1.room2Ptr, room2)) {
                                if (uint32_t levelId; READ(room2.levelPtr + 0x1F8, levelId)) {
                                    if (player.levelId != levelId) {
                                        player.levelChanged = true;
                                        player.levelId = levelId;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            paddr = unit.nextPtr;
        }
        playerPtr += 8;
    }
    if (mapPlayers_.empty()) {
        focusedPlayer_ = 0;
        return;
    }
    MapPlayer *currPlayer;
    if (!focusedPlayer_) {
        auto ite = mapPlayers_.begin();
        focusedPlayer_ = ite->first;
        currPlayer = &ite->second;
    } else {
        auto ite = mapPlayers_.find(focusedPlayer_);
        if (ite == mapPlayers_.end()) {
            ite = mapPlayers_.begin();
            focusedPlayer_ = ite->first;
        }
        currPlayer = &ite->second;
    }
    available_ = true;
    currPlayer_ = currPlayer;

    if (cfg->showMonsters) {
        mapMonsters_.clear();
        uint64_t baseAddr = baseAddr_ + HashTableBase + 8 * 0x80;
        auto showName = cfg->showMonsterName;
        auto showEnchant = cfg->showMonsterEnchant;
        auto showImmune = cfg->showMonsterImmune;
        auto showNormal = cfg->showNormalMonsters;
        auto monsSize = gamedata->monsters.size();
        for (int i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            if (!READ(baseAddr, paddr)) { break; }
            while (paddr) {
                UnitAny unit;
                do {
                    if (!READ(paddr, unit)) { break; }
                    if (unit.mode == 0 || unit.mode == 12 || unit.txtFileNo >= monsSize) { break; }
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
                    auto isUnique = (monData.flag & 0x0E) != 0;
                    auto &txtData = gamedata->monsters[unit.txtFileNo];
                    auto isNpc = txtData.second;
                    if (!showNormal && !isUnique && !isNpc) { break; }
                    DynamicPath path;
                    if (READ(unit.pathPtr, path)) {
                        auto &mon = mapMonsters_[unit.unitId];
                        mon.x = path.posX;
                        mon.y = path.posY;
                        if (!mon.txtFileNo) {
                            mon.txtFileNo = unit.txtFileNo;
                            mon.flag = monData.flag;
                        }
                        if (showName && (isUnique || isNpc)) {
                            /* Super unique */
                            if ((monData.flag & 2) && monData.uniqueNo < gamedata->superUniques.size()) {
                                auto ite2 = gamedata->strings.find(gamedata->superUniques[monData.uniqueNo]);
                                mon.name = ite2 != gamedata->strings.end() ? &ite2->second : nullptr;
                            } else {
                                auto ite2 = gamedata->strings.find(txtData.first);
                                mon.name = ite2 != gamedata->strings.end() ? &ite2->second : nullptr;
                                mon.isNpc = isNpc;
                            }
                        }
                        if (isUnique) {
                            int off = 0;
                            bool hasAura = false;
                            if (showEnchant) {
                                uint8_t id;
                                for (int n = 0; n < 9 && (id = monData.enchants[n]) != 0; ++n) {
                                    if (id == 30) { hasAura = true; continue; }
                                    const auto *str = enchantStrings[id];
                                    while (*str) {
                                        mon.enchants[off++] = *str++;
                                    }
                                }
                            }
                            if (showImmune || hasAura) {
                                if (StatList stats; READ(unit.statListPtr, stats)) {
                                    do {
                                        /* check if this is owner stat or aura */
                                        if (stats.ownerId == unit.unitId) {
                                            if (stats.stateNo) {
                                                if (hasAura) {
                                                    const wchar_t *str = auraStrings[stats.stateNo];
                                                    while (*str) {
                                                        mon.enchants[off++] = *str++;
                                                    }
                                                }
                                            } else if (showImmune) {
                                                static StatEx statEx[64];
                                                auto cnt = std::min(64u, stats.stat.statCount);
                                                if (readMemory64(handle_,
                                                                 stats.stat.statPtr,
                                                                 sizeof(StatEx) * cnt,
                                                                 statEx)) {
                                                    StatEx *st = statEx;
                                                    for (; cnt; --cnt, ++st) {
                                                        if (st->value < 100) { continue; }
                                                        if (auto statId = st->stateId; statId
                                                            < uint16_t(StatId::TotalCount)) {
                                                            if (auto mapping = statsMapping[statId]; mapping) {
                                                                const wchar_t *str = immunityStrings[mapping];
                                                                while (*str) {
                                                                    mon.enchants[off++] = *str++;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        if (!(stats.flag & 0x80000000u)) { break; }
                                        if (!stats.nextListEx || !READ(stats.nextListEx, stats)) { break; }
                                    } while (true);
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
        if (currPlayer->levelChanged) {
            mapObjects_.clear();
        }
        uint64_t baseAddr = baseAddr_ + HashTableBase + 8 * 0x100;
        for (int i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            if (!READ(baseAddr, paddr)) { break; }
            while (paddr) {
                UnitAny unit;
                if (!READ(paddr, unit)) { break; }
                if (unit.txtFileNo == 59 || unit.txtFileNo == 60 /* Portals */
                    || unit.mode != 2) {
                    auto ite = gamedata->objects[0].find(unit.txtFileNo);
                    if (ite != gamedata->objects[0].end()) {
                        auto type = ite->second.type;
                        switch (type) {
                        case TypePortal:
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

    mapEnabled_ = 0;

    mapPlayers_.clear();
    mapMonsters_.clear();
    mapObjects_.clear();
}
