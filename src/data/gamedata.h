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

namespace d2r {
struct UnitAny;
struct ItemData;
}

namespace data {

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

enum ELNG {
    LNG_enUS,
    LNG_zhTW,
    LNG_deDE,
    LNG_esES,
    LNG_frFR,
    LNG_itIT,
    LNG_koKR,
    LNG_plPL,
    LNG_esMX,
    LNG_jaJP,
    LNG_ptBR,
    LNG_ruRU,
    LNG_zhCN,
    LNG_MAX,
};

struct Data {
    using LngString = std::array<std::wstring, LNG_MAX>;
    std::unordered_map<std::string, LngString> strings;
    std::vector<std::pair<std::string, const LngString*>> levels, shrines, superUniques, items;
    std::vector<std::tuple<std::string, uint8_t, const LngString*>> monsters;
    std::unordered_map<std::string, uint32_t> itemIdByCode;
    std::unordered_map<int, std::tuple<EObjType, std::string, const LngString*, float, float>> objects[2];
    std::unordered_map<int, std::set<int>> guides;
    std::vector<LngString> mercNames;
};

extern void loadData();
extern uint16_t filterItem(const d2r::UnitAny *unit, const d2r::ItemData *item, uint32_t sockets);

extern const Data *gamedata;

}
