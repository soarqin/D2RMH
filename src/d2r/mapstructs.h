/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <array>
#include <string>
#include <cstdint>

namespace d2r {

struct MapPlayer {
    uint32_t act;
    uint32_t seed;
    uint32_t levelId;
    int posX, posY;
    char name[16];
    uint32_t classId;
    uint16_t level;
    uint16_t party;
    uint8_t difficulty;
    bool levelChanged;
    std::array<int32_t, 16> stats;
    uint64_t skillPtr;
};
struct MapMonster {
    int x, y;
    const std::array<std::wstring, 13> *name;
    wchar_t enchants[32];
    uint8_t flag;
    bool isNpc;
    bool isUnique;
};
struct MapObject {
    int x, y;
    const std::array<std::wstring, 13> *name;
    uint8_t type;
    uint8_t flag;
    float w, h;
};
struct MapItem {
    int x, y;
    const std::array<std::wstring, 13> *name;
    uint8_t flag;
    uint8_t color;
};

}
