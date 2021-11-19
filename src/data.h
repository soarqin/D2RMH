/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <array>
#include <set>
#include <vector>
#include <unordered_map>
#include <string>

enum EObjType {
    TypeNone,
    TypeWayPoint,
    TypePortal,
    TypeChest,
    TypeQuest,
    TypeShrine,
    TypeWell,
    TypeMonster,
    TypeUniqueMonster,
    TypeNpc,
    TypeDoor,
    TypeMax,
};

struct Data {
    using LngString = std::array<std::wstring, 13>;
    std::unordered_map<std::string, LngString> strings;
    std::vector<std::pair<std::string, const LngString*>> levels, shrines, superUniques, items;
    std::vector<std::tuple<std::string, bool, const LngString*>> monsters;
    std::unordered_map<std::string, uint32_t> itemIdByCode;
    std::unordered_map<int, std::tuple<EObjType, std::string, const LngString*, float, float>> objects[2];
    std::unordered_map<int, std::set<int>> guides;
};

struct UnitAny;
struct ItemData;

extern void loadData();
extern uint8_t filterItem(const UnitAny *unit, const ItemData *item, uint32_t sockets);

extern const Data *gamedata;
