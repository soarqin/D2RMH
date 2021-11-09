/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <string>

struct Cfg {
    std::string d2Path = ".";
    std::string fontFilePath = R"(C:\Windows\Fonts\Arial.ttf)";
    int fontSize = 12;
    std::string language = "enUS";

    int show = 0;
    int fullLine = 0;
    int position = 1;
    float scale = 1;
    int mapCentered = 0;
    uint8_t alpha = 170;
#define RGBA(r, g, b, a) (uint32_t(r) | (uint32_t(g) << 8) | (uint32_t(b) << 16) | (uint32_t(a) << 24))
    uint32_t walkableColor = RGBA(50, 50, 50, 255);
    uint32_t textColor = RGBA(255, 255, 255, 255);
    uint32_t playerInnerColor = RGBA(255, 128, 128, 255);
    uint32_t playerOuterColor = RGBA(51, 255, 255, 255);
    uint32_t lineColor = RGBA(204, 204, 204, 255);
    uint32_t waypointColor = RGBA(153, 153, 255, 255);
    uint32_t portalColor = RGBA(255, 153, 255, 255);
    uint32_t chestColor = RGBA(255, 104, 104, 255);
    uint32_t questColor = RGBA(104, 104, 255, 255);
    uint32_t shrineColor = RGBA(255, 51, 178, 255);
    uint32_t wellColor = RGBA(51, 51, 255, 255);
    uint32_t monsterColor = RGBA(255, 0, 0, 255);
#undef RGBA

    int showMonsters = 1;
    int showObjects = 1;
    int showMonsterName = 1;
    int showMonsterEnchant = 1;
    int showMonsterImmune = 1;
};

extern void loadCfg(const std::string &filename = "D2RMH.ini");

extern const Cfg *cfg;
