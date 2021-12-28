/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "cfg.h"

#include "util/util.h"

#include "ini.h"

#include <fstream>
#include <algorithm>
#include <cstring>

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#endif

static Cfg sCfg;
const Cfg *cfg = &sCfg;

#define LOADVAL(n, m) else if (!strcmp(name, #n)) { sCfg.m = value; }
#define LOADVALW(n, m) else if (!strcmp(name, #n)) { sCfg.m = util::utf8toucs4(value); }
#define LOADVALN(n, m) else if (!strcmp(name, #n)) { sCfg.m = decltype(sCfg.m)(strtol(value, nullptr, 0)); }
#define LOADVALF(n, m) else if (!strcmp(name, #n)) { sCfg.m = strtof(value, nullptr); }
#define LOADVALC(n, m) else if (!strcmp(name, #n)) { sCfg.m = calcColor(value); }

inline uint32_t calcColor(const char *value) {
    uint32_t c = strtoul(value, nullptr, 0);
    const char *tok = strchr(value, ',');
    if (tok) {
        c |= strtoul(++tok, nullptr, 0) << 8;
        tok = strchr(tok, ',');
        if (tok) {
            c |= strtoul(++tok, nullptr, 0) << 16;
            tok = strchr(tok, ',');
            if (tok) {
                c |= strtoul(++tok, nullptr, 0) * sCfg.alpha / 255 << 24;
            } else {
                c |= 0xFF000000u;
            }
        }
    }
    return c;
}

void loadCfg(const std::string &filename) {
    sCfg = Cfg {};
    int section = -1;
    ini_parse(filename.c_str(), [](void* user, const char* section,
                                   const char* name, const char* value)->int {
        if (!name) {
            if (!strcmp(section, "main")) { *(int*)user = 0; }
            else if (!strcmp(section, "ui")) { *(int*)user = 1; }
            else if (!strcmp(section, "enchants")) { *(int*)user = 2; }
            else if (!strcmp(section, "sound")) { *(int*)user = 3; }
            else { *(int*)user = -1; }
            return 1;
        }
        switch (*(int*)user) {
        case 0:
            if (false) {}
            LOADVALW(d2_path, d2Path)
            LOADVAL(font_file_path, fontFilePath)
            LOADVALN(font_size, fontSize)
            LOADVALN(msg_font_size, msgFontSize)
            LOADVAL(language, language)
            break;
        case 1:
            if (false) {}
            LOADVALN(fps, fps)
            LOADVALN(show, show)
            LOADVALN(draw_on_game_bar, drawOnGameBar)
            LOADVALN(panel_mask, panelMask)
            LOADVALN(full_line, fullLine)
            LOADVALN(line_style, lineStyle)
            LOADVALN(position, position)
            LOADVAL(map_area, mapArea)
            LOADVALF(scale, scale)
            LOADVALN(map_centered, mapCentered)

            LOADVALN(alpha, alpha)
            LOADVALN(neighbour_map_bounds, neighbourMapBounds)
            LOADVALC(walkable_color, walkableColor)
            LOADVALC(edge_color, edgeColor)
            LOADVALC(text_color, textColor)
            LOADVALC(player_inner_color, playerInnerColor)
            LOADVALC(player_outer_color, playerOuterColor)
            LOADVALC(non_party_player_inner_color, nonPartyPlayerInnerColor)
            LOADVALC(non_party_player_outer_color, nonPartyPlayerOuterColor)
            LOADVALC(line_color, lineColor)
            LOADVALC(waypoint_color, waypointColor)
            LOADVALC(portal_color, portalColor)
            LOADVALC(chest_color, chestColor)
            LOADVALC(quest_color, questColor)
            LOADVALC(shrine_color, shrineColor)
            LOADVALC(well_color, wellColor)
            LOADVALC(unique_monster_color, uniqueMonsterColor)
            LOADVALC(monster_color, monsterColor)
            LOADVALC(npc_color, npcColor)
            LOADVALC(door_color, doorColor)
            LOADVALC(msg_bg_color, msgBgColor)
            LOADVAL(msg_position, msgPosition)
            LOADVALW(text_panel_pattern, panelPattern)
            LOADVAL(text_panel_position, panelPosition)

            LOADVALN(show_player_names, showPlayerNames)
            LOADVALN(show_npc_names, showNpcNames)
            LOADVALN(show_objects, showObjects)
            LOADVALN(show_items, showItems)
            LOADVALN(show_npc_name, showNpcNames)
            LOADVALN(show_monsters, showMonsters)
            LOADVALN(show_monster_names, showMonsterNames)
            LOADVALN(show_monster_enchants, showMonsterEnchants)
            LOADVALN(show_monster_immunities, showMonsterImmunities)
            /* backward compatibility */
            LOADVALN(show_normal_monsters, showNormalMonsters)
            LOADVALN(show_monster_name, showMonsterNames)
            LOADVALN(show_monster_enchant, showMonsterEnchants)
            LOADVALN(show_monster_immune, showMonsterImmunities)
            LOADVALF(object_size_minimal, objectSizeMinimal)
            break;
        case 2:
            if (false) {}
            LOADVALW(extra_strong, encTxtExtraStrong)
            LOADVALW(extra_fast, encTxtExtraFast)
            LOADVALW(cursed, encTxtCursed)
            LOADVALW(magic_resistant, encTxtMagicResistant)
            LOADVALW(fire_enchanted, encTxtFireEnchanted)
            LOADVALW(ligntning_enchanted, encTxtLigntningEnchanted)
            LOADVALW(cold_enchanted, encTxtColdEnchanted)
            LOADVALW(mana_burn, encTxtManaBurn)
            LOADVALW(teleportation, encTxtTeleportation)
            LOADVALW(spectral_hit, encTxtSpectralHit)
            LOADVALW(stone_skin, encTxtStoneSkin)
            LOADVALW(multiple_shots, encTxtMultipleShots)
            LOADVALW(fanatic, encTxtFanatic)
            LOADVALW(berserker, encTxtBerserker)

            LOADVALW(might_aura, MightAura)
            LOADVALW(holyFire_aura, HolyFireAura)
            LOADVALW(blessedAim_aura, BlessedAimAura)
            LOADVALW(holyFreeze_aura, HolyFreezeAura)
            LOADVALW(holyShock_aura, HolyShockAura)
            LOADVALW(conviction_aura, ConvictionAura)
            LOADVALW(fanaticism_aura, FanaticismAura)

            LOADVALW(physical_immunity, encTxtPhysicalImmunity)
            LOADVALW(magic_immunity, encTxtMagicImmunity)
            LOADVALW(fire_immunity, encTxtFireImmunity)
            LOADVALW(lightning_immunity, encTxtLightningImmunity)
            LOADVALW(cold_immunity, encTxtColdImmunity)
            LOADVALW(poison_immunity, encTxtPoisonImmunity)
            break;
        case 3: {
            if (strncmp(name, "sound[", 6) != 0) { break; }
            auto index = size_t(strtol(name + 6, nullptr, 0));
            if (!index) { break; }
            if (index >= sCfg.sounds.size()) { sCfg.sounds.resize(index + 1); }
            auto vlen = strlen(value);
            if (!strcasecmp(value + vlen - 4, ".wav")) {
                sCfg.sounds[index] = {util::utf8toucs4(value), false};
            } else {
                sCfg.sounds[index] = {util::utf8toucs4(value), true};
            }
            break;
        }
        default:
            break;
        }
        return 1;
    }, &section);
    sCfg.scale = std::clamp(sCfg.scale, 1.f, 4.f);
    if (sCfg.showNormalMonsters) { sCfg.showMonsters = 2; }
    if (sCfg.lineStyle == 0 && sCfg.fullLine > 0) {
        sCfg.lineStyle = sCfg.fullLine;
    }
    if (!sCfg.mapArea.empty()) {
        auto vec = util::splitString(sCfg.mapArea, ',');
        if (vec.size() > 1) {
            sCfg.mapAreaW = std::clamp(strtof(vec[0].c_str(), nullptr), 0.f, 1.f);
            sCfg.mapAreaH = std::clamp(strtof(vec[1].c_str(), nullptr), 0.f, 1.f);
        } else {
            sCfg.mapAreaW = sCfg.mapAreaH = std::clamp(strtof(sCfg.mapArea.c_str(), nullptr), 0.f, 1.f);
        }
    }
    if (!sCfg.msgPosition.empty()) {
        auto vec = util::splitString(sCfg.msgPosition, ',');
        auto sz = vec.size();
        if (sz > 0) {
            sCfg.msgPositionX = std::clamp(strtof(vec[0].c_str(), nullptr), 0.f, 1.f) - .5f;
        }
        if (sz > 1) {
            sCfg.msgPositionY = std::clamp(strtof(vec[1].c_str(), nullptr), 0.f, 1.f) - .5f;
        }
        if (sz > 2) {
            sCfg.msgAlign = std::clamp(int(strtol(vec[2].c_str(), nullptr, 0)), 0, 2);
        }
    }
    if (!sCfg.panelPattern.empty()) {
        sCfg.panelPatterns.clear();
        typename std::wstring::size_type last = 0;
        while (true) {
            auto pos = sCfg.panelPattern.find(L"{newline}", last);
            auto pos2 = sCfg.panelPattern.find(L"{n}", last);
            int sepLen;
            if (pos > pos2) {
                pos = pos2;
                sepLen = 3;
            } else {
                sepLen = 9;
            }
            if (pos == std::wstring::npos) {
                break;
            }
            sCfg.panelPatterns.emplace_back(sCfg.panelPattern.substr(last, pos - last));
            last = pos + sepLen;
        }
        if (last < sCfg.panelPattern.size()) {
            sCfg.panelPatterns.emplace_back(sCfg.panelPattern.substr(last));
        }
    }
    if (!sCfg.panelPosition.empty()) {
        auto vec = util::splitString(sCfg.panelPosition, ',');
        auto sz = vec.size();
        if (sz > 0) {
            sCfg.panelPositionX = std::clamp(strtof(vec[0].c_str(), nullptr), 0.f, 1.f) - .5f;
        }
        if (sz > 1) {
            sCfg.panelPositionY = std::clamp(strtof(vec[1].c_str(), nullptr), 0.f, 1.f) - .5f;
        }
        if (sz > 2) {
            sCfg.panelAlign = std::clamp(int(strtol(vec[2].c_str(), nullptr, 0)), 0, 2);
        }
    }
    for (auto *color:
        {&sCfg.walkableColor, &sCfg.edgeColor, &sCfg.textColor, &sCfg.playerInnerColor, &sCfg.playerOuterColor,
         &sCfg.lineColor, &sCfg.waypointColor, &sCfg.portalColor, &sCfg.chestColor, &sCfg.questColor, &sCfg.shrineColor,
         &sCfg.wellColor, &sCfg.uniqueMonsterColor, &sCfg.monsterColor, &sCfg.npcColor, &sCfg.doorColor,
         &sCfg.msgBgColor,}) {
        *color = (((*color >> 24) * sCfg.alpha / 255) << 24) | (*color & 0xFFFFFFu);
    }
}
