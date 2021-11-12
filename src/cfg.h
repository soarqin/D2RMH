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

    int fps = -1;
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
    uint32_t portalColor = RGBA(255, 255, 153, 255);
    uint32_t chestColor = RGBA(255, 104, 104, 255);
    uint32_t questColor = RGBA(104, 104, 255, 255);
    uint32_t shrineColor = RGBA(255, 51, 178, 255);
    uint32_t wellColor = RGBA(51, 51, 255, 255);
    uint32_t monsterColor = RGBA(255, 0, 0, 255);
    uint32_t npcColor = RGBA(160, 160, 160, 255);
#undef RGBA

    int showMonsters = 1;
    int showNormalMonsters = 0;
    int showObjects = 1;
    int showMonsterName = 1;
    int showMonsterEnchant = 1;
    int showMonsterImmune = 1;

    std::string encTxtExtraStrong = "S";
    std::string encTxtExtraFast = "F";
    std::string encTxtCursed = "{2}C";
    std::string encTxtMagicResistant = "M";
    std::string encTxtFireEnchanted = "{1}FE";
    std::string encTxtLigntningEnchanted = "{9}LE";
    std::string encTxtColdEnchanted = "{3}CE";
    std::string encTxtManaBurn = "{3}MB";
    std::string encTxtTeleportation = "T";
    std::string encTxtSpectralHit = "H";
    std::string encTxtStoneSkin = "{4}SS";
    std::string encTxtMultipleShots = "{12}MS";
    std::string encTxtFanatic = "{11}F";
    std::string encTxtBerserker = "{4}B";

    std::string MightAura = "{4}A";
    std::string HolyFireAura = "{1}A";
    std::string BlessedAimAura = "A";
    std::string HolyFreezeAura = "{3}A";
    std::string HolyShockAura = "{9}A";
    std::string ConvictionAura = "{11}A";
    std::string FanaticismAura = "{5}A";

    std::string encTxtPhysicalImmunity = "{4}i";
    std::string encTxtMagicImmunity = "{8}i";
    std::string encTxtFireImmunity = "{1}i";
    std::string encTxtLightningImmunity = "{9}i";
    std::string encTxtColdImmunity = "{3}i";
    std::string encTxtPoisonImmunity = "{2}i";
};

extern void loadCfg(const std::string &filename = "D2RMH.ini");

extern const Cfg *cfg;
