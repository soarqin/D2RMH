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
    float fontSize = 12;
    std::string language = "enUS";

    int show = 0;
    int fullLine = 0;
    int position = 1;
    float scale = 1;
    int mapCentered = 0;
    uint8_t alpha = 0;
    uint32_t walkableColor = 0;
    uint32_t textColor = 0;
    uint32_t playerInnerColor = 0;
    uint32_t playerOuterColor = 0;
    uint32_t lineColor = 0;
    uint32_t waypointColor = 0;
    uint32_t portalColor = 0;
    uint32_t chestColor = 0;
    uint32_t questColor = 0;
    uint32_t wellColor = 0;
};

extern void loadCfg(const std::string &filename = "D2RMH.ini");

extern const Cfg *cfg;
