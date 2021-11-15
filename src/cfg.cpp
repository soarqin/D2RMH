/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "cfg.h"

#include "util.h"
#include "ini.h"

#include <algorithm>
#include <cstring>

static Cfg sCfg;
const Cfg *cfg = &sCfg;

#define LOADVAL(n, m) else if (!strcmp(name, #n)) { sCfg.m = value; }
#define LOADVALN(n, m) else if (!strcmp(name, #n)) { sCfg.m = strtoul(value, nullptr, 0); }
#define LOADVALF(n, m) else if (!strcmp(name, #n)) { sCfg.m = strtof(value, nullptr); }
#define LOADVALC(n, m) else if (!strcmp(name, #n)) { sCfg.m = calcColor(value); }

inline uint32_t calcColor(const char *value) {
    uint32_t c = 0xFF000000u | strtoul(value, nullptr, 0);
    const char *tok = strchr(value, ',');
    if (!tok) { return c; }
    c |= strtoul(++tok, nullptr, 0) << 8;
    tok = strchr(tok, ',');
    if (!tok) { return c; }
    c |= strtoul(++tok, nullptr, 0) << 16;
    return c;
}

void loadCfg(const std::string &filename) {
    int section = -1;
    ini_parse(filename.c_str(), [](void* user, const char* section,
                                   const char* name, const char* value)->int {
        if (!name) {
            if (!strcmp(section, "main")) { *(int*)user = 0; }
            else if (!strcmp(section, "ui")) { *(int*)user = 1; }
            else if (!strcmp(section, "enchants")) { *(int*)user = 2; }
            else { *(int*)user = -1; }
            return 1;
        }
        switch (*(int*)user) {
        case 0:
            if (false) {}
            LOADVAL(d2_path, d2Path)
            LOADVAL(font_file_path, fontFilePath)
            LOADVALN(font_size, fontSize)
            LOADVALN(msg_font_size, msgFontSize)
            LOADVAL(language, language)
            break;
        case 1:
            if (false) {}
            LOADVALN(fps, fps)
            LOADVALN(show, show)
            LOADVALN(full_line, fullLine)
            LOADVALN(position, position)
            LOADVALF(scale, scale)
            LOADVALN(map_centered, mapCentered)

            LOADVALN(alpha, alpha)
            LOADVALC(walkable_color, walkableColor)
            LOADVALC(text_color, textColor)
            LOADVALC(player_inner_color, playerInnerColor)
            LOADVALC(player_outer_color, playerOuterColor)
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
            LOADVALC(msg_bg_color, msgBgColor)
            LOADVAL(msg_position, msgPosition)

            LOADVALN(show_player_names, showPlayerNames)
            LOADVALN(show_monsters, showMonsters)
            LOADVALN(show_normal_monsters, showNormalMonsters)
            LOADVALN(show_objects, showObjects)
            LOADVALN(show_items, showItems)
            LOADVALN(show_monster_name, showMonsterName)
            LOADVALN(show_monster_enchant, showMonsterEnchant)
            LOADVALN(show_monster_immune, showMonsterImmune)
            break;
        case 2:
            if (false) {}
            LOADVAL(extra_strong, encTxtExtraStrong)
            LOADVAL(extra_fast, encTxtExtraFast)
            LOADVAL(cursed, encTxtCursed)
            LOADVAL(magic_resistant, encTxtMagicResistant)
            LOADVAL(fire_enchanted, encTxtFireEnchanted)
            LOADVAL(ligntning_enchanted, encTxtLigntningEnchanted)
            LOADVAL(cold_enchanted, encTxtColdEnchanted)
            LOADVAL(mana_burn, encTxtManaBurn)
            LOADVAL(teleportation, encTxtTeleportation)
            LOADVAL(spectral_hit, encTxtSpectralHit)
            LOADVAL(stone_skin, encTxtStoneSkin)
            LOADVAL(multiple_shots, encTxtMultipleShots)
            LOADVAL(fanatic, encTxtFanatic)
            LOADVAL(berserker, encTxtBerserker)

            LOADVAL(might_aura, MightAura)
            LOADVAL(holyFire_aura, HolyFireAura)
            LOADVAL(blessedAim_aura, BlessedAimAura)
            LOADVAL(holyFreeze_aura, HolyFreezeAura)
            LOADVAL(holyShock_aura, HolyShockAura)
            LOADVAL(conviction_aura, ConvictionAura)
            LOADVAL(fanaticism_aura, FanaticismAura)

            LOADVAL(physical_immunity, encTxtPhysicalImmunity)
            LOADVAL(magic_immunity, encTxtMagicImmunity)
            LOADVAL(fire_immunity, encTxtFireImmunity)
            LOADVAL(lightning_immunity, encTxtLightningImmunity)
            LOADVAL(cold_immunity, encTxtColdImmunity)
            LOADVAL(poison_immunity, encTxtPoisonImmunity)
            break;
        default:
            break;
        }
        return 1;
    }, &section);
    sCfg.scale = std::clamp(sCfg.scale, 1.f, 4.f);
    auto vec = splitString(sCfg.msgPosition, ',');
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
