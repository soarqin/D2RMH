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
    std::wstring d2Path = L".";
    std::string fontFilePath = R"(C:\Windows\Fonts\Arial.ttf)";
    int fontSize = 14;
    int msgFontSize = 24;
    std::string language = "enUS";

    int fps = -1;
    int show = 0;
    uint32_t panelMask = 0x7F;
    int fullLine = 0;
    int position = 2;
    std::string mapArea;
    float mapAreaW = 1.f;
    float mapAreaH = 1.f;
    float scale = 2.f;
    int mapCentered = 1;
    uint8_t alpha = 170;
#define RGBA(r, g, b, a) (uint32_t(r) | (uint32_t(g) << 8) | (uint32_t(b) << 16) | (uint32_t(a) << 24))
    uint32_t walkableColor = RGBA(20, 20, 20, 255);
    uint32_t edgeColor = RGBA(128, 128, 128, 255);
    uint32_t textColor = RGBA(255, 255, 255, 255);
    uint32_t playerInnerColor = RGBA(255, 128, 128, 255);
    uint32_t playerOuterColor = RGBA(51, 255, 255, 255);
    uint32_t lineColor = RGBA(204, 204, 204, 255);
    uint32_t waypointColor = RGBA(153, 153, 255, 255);
    uint32_t portalColor = RGBA(255, 255, 102, 255);
    uint32_t chestColor = RGBA(255, 104, 104, 255);
    uint32_t questColor = RGBA(104, 104, 255, 255);
    uint32_t shrineColor = RGBA(255, 51, 178, 255);
    uint32_t wellColor = RGBA(51, 51, 255, 255);
    uint32_t uniqueMonsterColor = RGBA(192, 166, 130, 255);
    uint32_t monsterColor = RGBA(255, 0, 0, 255);
    uint32_t npcColor = RGBA(160, 160, 160, 255);
    uint32_t doorColor = RGBA(80, 255, 80, 255);
    uint32_t msgBgColor = RGBA(1, 1, 1, 255);
#undef RGBA
    std::string msgPosition;
    float msgPositionX = .95f - .5f;
    float msgPositionY = .25f - .5f;
    int msgAlign = 2;

    int showPlayerNames = 1;
    int showNpcNames = 1;
    int showObjects = 1;
    int showItems = 1;
    int showMonsters = 1;
    int showNormalMonsters = 0;
    int showMonsterNames = 0;
    int showMonsterEnchants = 1;
    int showMonsterImmunities = 1;

    std::wstring encTxtExtraStrong = L"S";
    std::wstring encTxtExtraFast = L"F";
    std::wstring encTxtCursed = L"{2}C";
    std::wstring encTxtMagicResistant = L"M";
    std::wstring encTxtFireEnchanted = L"{1}FE";
    std::wstring encTxtLigntningEnchanted = L"{9}LE";
    std::wstring encTxtColdEnchanted = L"{3}CE";
    std::wstring encTxtManaBurn = L"{3}MB";
    std::wstring encTxtTeleportation = L"T";
    std::wstring encTxtSpectralHit = L"H";
    std::wstring encTxtStoneSkin = L"{4}SS";
    std::wstring encTxtMultipleShots = L"{12}MS";
    std::wstring encTxtFanatic = L"{11}F";
    std::wstring encTxtBerserker = L"{4}B";

    std::wstring MightAura = L"{4}A";
    std::wstring HolyFireAura = L"{1}A";
    std::wstring BlessedAimAura = L"A";
    std::wstring HolyFreezeAura = L"{3}A";
    std::wstring HolyShockAura = L"{9}A";
    std::wstring ConvictionAura = L"{11}A";
    std::wstring FanaticismAura = L"{5}A";

    std::wstring encTxtPhysicalImmunity = L"{4}i";
    std::wstring encTxtMagicImmunity = L"{8}i";
    std::wstring encTxtFireImmunity = L"{1}i";
    std::wstring encTxtLightningImmunity = L"{9}i";
    std::wstring encTxtColdImmunity = L"{3}i";
    std::wstring encTxtPoisonImmunity = L"{2}i";
};

extern void loadCfg(const std::string &filename = "D2RMH.ini");

extern const Cfg *cfg;
