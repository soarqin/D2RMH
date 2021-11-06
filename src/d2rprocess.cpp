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

#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <cstdio>

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
}

D2RProcess::~D2RProcess() {
    if (handle_) { CloseHandle(handle_); }
}

struct DrlgRoom2 {
    uint64_t unk0[2];
    /* 0x10  DrlgRoom2 **pRoomsNear */
    uint64_t roomsNearListPtr;
    uint64_t unk1[5];
    /* 0x40  DWORD *levelDef */
    uint64_t levelPresetPtr;
    /* 0x48  DrlgRoom2 *pNext */
    uint64_t nextPtr;
    /* 0x50 */
    uint16_t roomsNear;
    uint16_t unk2;
    uint32_t roomTiles;
    /* 0x58  DrlgRoom1 *room1 */
    uint64_t room1Ptr;
    /* 0x60  DWORD dwPosX; DWORD dwPosY; DWORD dwSizeX; DWORD dwSizeY; */
    uint32_t posX, posY, sizeX, sizeY;
    /* 0x70 */
    uint32_t unk3;
    uint32_t presetType;
    /* 0x78  RoomTile *pRoomTiles */
    uint64_t roomTilesPtr;
    uint64_t unk4[2];
    /* 0x90  DrlgLevel *pLevel */
    uint64_t levelPtr;
    /* 0x98  PresetUnit *pPresetUnits */
    uint64_t presetUnitsPtr;
};

struct DrlgRoom1 {
    /* 0x00  DrlgRoom1 **pRoomsNear; */
    uint64_t roomsNearListPtr;
    uint64_t unk0[2];
    /* 0x18  DrlgRoom2 *pRoom2; */
    uint64_t room2Ptr;
    uint64_t unk1[4];
    /* 0x40 */
    uint32_t roomsNear;
    uint32_t unk2;
    /* 0x48  DrlgAct *pAct */
    uint64_t actAddr;
    uint64_t unk3[11];
    /* 0xA8  UnitAny *pUnitFirst */
    uint64_t unitFirstAddr;
    /* 0xB0  DrlgRoom1 *pNext */
    uint64_t nextPtr;
};

struct DynamicPath {
    uint16_t offsetX;
    uint16_t posX;
    uint16_t offsetY;
    uint16_t posY;
    uint32_t mapPosX;
    uint32_t mapPosY;
    uint32_t targetX;
    uint32_t targetY;
    uint32_t unk0[2];
    /* 0x20  DrlgRoom1 *pRoom1 */
    uint64_t room1Ptr;
};

struct StaticPath {
    /* DrlgRoom1 *pRoom1; */
    uint64_t room1Ptr;
    uint32_t mapPosX;
    uint32_t dwMapPosY;
    uint32_t posX;
    uint32_t posY;
};

struct DrlgAct {
    uint64_t unk0[2];
    uint32_t unk1;
    uint32_t seed;
    /* 0x18 DrlgRoom1 *room1 */
    uint64_t room1Ptr;
    uint32_t actId;
    uint32_t unk2;
    uint64_t unk3[9];
    /* DrlgMisc *misc */
    uint64_t miscPtr;
};

struct UnitAny {
    uint32_t unitType;
    uint32_t txtFileNo;
    uint32_t unitId;
    uint32_t mode;
    /* 0x10
    union {
        PlayerData *pPlayerData;
        MonsterData *pMonsterData;
        ObjectData *pObjectData;
        ItemData *pItemData;
        MissileData *pMissileData;
    };
     */
    uint64_t unionPtr;
    uint64_t unk0;
    /* 0x20  DrlgAct *pAct */
    uint64_t actPtr;
    uint64_t seed;
    /* 0x30 */
    uint64_t initSeed;
    /* 0x38  for Player/Monster: DynamicPath *pPath
     *       for Object:         StaticPath  *pPath */
    uint64_t pathPtr;
    /* 0x40 */
    uint32_t unk2[8];
    /* 0x60 */
    uint32_t gfxFrame;
    uint32_t frameRemain;
    /* 0x68 */
    uint32_t frameRate;
    uint32_t unk3;
    /* 0x70
    uint8_t *pGfxUnk;
    uint32_t *pGfxInfo;
     */
    uint64_t gfxUnkPtr;
    uint64_t gfxInfoPtr;
    /* 0x80 */
    uint64_t unk4;
    /* 0x88 StatList *pStats */
    uint64_t statsPtr;
    /* 0x90 Inventory1 *pInventory */
    uint64_t inventoryPtr;
    uint64_t unk5[23];
    uint64_t nextPtr;
};

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
                    break;
                }
                paddr = unit.nextPtr;
            }
            if (playerHashOffset_) {
                break;
            }
            playerPtr += 8;
        }
    }

    uint64_t addr;
    if (!READ(playerHashOffset_, addr) || !addr) {
        playerHashOffset_ = 0;
        return;
    }
    READ(baseAddr_ + MapEnabledAddr, mapEnabled_);
    READ(playerHashOffset_, addr);

    UnitAny unit;
    if (!READ(addr, unit)) {
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
        for (int i = 0; i < 0x80; ++i) {
            uint64_t paddr;
            if (!READ(baseAddr, paddr)) { break; }
            while (paddr) {
                UnitAny unit;
                if (!READ(paddr, unit)) { break; }
                if (unit.mode != 0 && unit.mode != 12) {
                    uint8_t flag;
                    READ(unit.unionPtr + 0x1A, flag);
                    if (flag & 0x1E) {
                        DynamicPath path;
                        if (READ(unit.pathPtr, path)) {
                            auto &mon = mapMonsters_[unit.unitId];
                            mon.x = path.posX;
                            mon.y = path.posY;
                            if (!mon.txtFileNo) {
                                mon.txtFileNo = unit.txtFileNo;
                                mon.flag = flag;
                            }
                        }
                    }
                }
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
                    if (ite != gamedata->objects[0].end() && ite->second.type == TypeShrine) {
                        uint8_t flag;
                        READ(unit.unionPtr + 8, flag);
                        StaticPath path;
                        if (READ(unit.pathPtr, path)) {
                            auto &obj = mapObjects_[unit.unitId];
                            if (!obj.txtFileNo) {
                                obj.txtFileNo = unit.txtFileNo;
                                obj.type = ite->second.type;
                                const std::string &name =
                                    flag < gamedata->shrines.size() ? gamedata->shrines[flag] : ite->second.name;
                                auto ite2 = gamedata->strings.find(name);
                                obj.name = ite2 != gamedata->strings.end() ? &ite2->second : nullptr;
                                obj.x = path.posX;
                                obj.y = path.posY;
                                obj.flag = flag;
                            }
                        }
                    }
                } else {
                    mapObjects_.erase(unit.unitId);
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
