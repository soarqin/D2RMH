/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "mapstructs.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace d2r {

struct UnitAny;
struct DrlgRoom1;
struct StatList;
struct Skill;

class ProcessData final {
public:
    ProcessData();
    ~ProcessData();
    void resetData();
    void updateData();
    void updateOffset();
    Skill *getSkill(uint16_t id);

private:
    void readRosters();
    void readUnitHashTable(uint64_t addr, const std::function<void(const UnitAny &)> &callback);
    void readStatList(uint64_t addr, uint32_t unitId, const std::function<void(const StatList &)> &callback);
    void readPlayerStats(const UnitAny &unit, const std::function<void(uint16_t, int32_t)> &callback);
    void readUnit(const UnitAny &unit);
    void readUnitPlayer(const UnitAny &unit);
    void readUnitMonster(const UnitAny &unit);
    void readUnitObject(const UnitAny &unit);
    void readUnitItem(const UnitAny &unit);
    void readGameInfo();

public:
    void *handle = nullptr;
    void *hwnd = nullptr;
    void *hook = nullptr;

    uint64_t baseAddr = 0;
    uint64_t baseSize = 0;

    uint64_t hashTableBaseAddr = 0;
    uint64_t uiBaseAddr = 0;
    uint64_t isExpansionAddr = 0;
    uint64_t rosterDataAddr = 0;
    uint64_t gameInfoAddr = 0;

    uint8_t mapEnabled = 0;
    uint32_t panelEnabled = 0;

    uint32_t focusedPlayer = 0;
    const MapPlayer *currPlayer = nullptr;

    std::wstring gameName, gamePass, region, gameIP;

    uint32_t realTombLevelId = 0;
    uint32_t superUniqueTombLevelId = 0;

    std::unordered_map<uint32_t, MapPlayer> mapPlayers;
    std::vector<MapMonster> mapMonsters;
    std::unordered_map<uint32_t, MapObject> mapObjects;
    std::vector<MapItem> mapItems;
    std::unordered_set<uint32_t> knownItems;
};

}
