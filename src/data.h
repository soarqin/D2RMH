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
#include <map>
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
    TypeMax,
};

struct Data {
    struct ObjType {
        EObjType type;
        std::string name;
    };
    std::unordered_map<std::string, std::array<std::string, 13>> strings;
    std::map<int, std::string> levels;
    std::map<int, ObjType> objects[2];
    std::map<int, std::set<int>> guides;
};

extern void loadData();

extern const Data *gamedata;
